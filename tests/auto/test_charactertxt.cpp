#include <string>

#include <stdutau/charactertxt.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_charactertxt)

namespace {

    CharacterTxt parse(const std::string &text) {
        CharacterTxt character;
        BOOST_REQUIRE(character.read(text));
        return character;
    }

    std::string written(const CharacterTxt &character) {
        return character.write();
    }

    // The output of the library, regardless of the input. A UTAU file uses CRLF on every platform.
    std::string crlf(const std::string &text) {
        std::string out;
        for (char c : text) {
            if (c == '\n') {
                out += '\r';
            }
            out += c;
        }
        return out;
    }

}

BOOST_AUTO_TEST_CASE(test_read) {
    auto character = parse("name=New Geping UTAU Database\n"
                           "image=avatar.jpg\n"
                           "sample=a.wav\n"
                           "author=Ge Ping\n"
                           "web=http://utau.vocalover.com/\n");

    BOOST_CHECK_EQUAL(character.name, "New Geping UTAU Database");
    BOOST_CHECK_EQUAL(character.image, "avatar.jpg");
    BOOST_CHECK_EQUAL(character.sample, "a.wav");
    BOOST_CHECK_EQUAL(character.author, "Ge Ping");
    BOOST_CHECK_EQUAL(character.web, "http://utau.vocalover.com/");
    BOOST_CHECK(character.extraLines.empty());
}

// UTAU displays a line containing a colon as part of the character profile, so a real voice bank
// contains lines that are not entries. They are content, and losing them would delete author
// data.
BOOST_AUTO_TEST_CASE(test_a_line_that_is_not_an_entry_is_kept) {
    auto character = parse("name=uta\n"
                           "\n"
                           "Version:1.0\n"
                           "\xe8\xaa\xb0: \xe3\x81\x86\xe3\x81\x9f\n");

    BOOST_CHECK_EQUAL(character.name, "uta");
    BOOST_REQUIRE_EQUAL(character.extraLines.size(), 3);
    BOOST_CHECK_EQUAL(character.extraLines.at(0), "");
    BOOST_CHECK_EQUAL(character.extraLines.at(1), "Version:1.0");
    BOOST_CHECK_EQUAL(character.extraLines.at(2), "\xe8\xaa\xb0: \xe3\x81\x86\xe3\x81\x9f");
}

// An entry without a dedicated member is handled the same way and is written back unchanged
// rather than dropped.
BOOST_AUTO_TEST_CASE(test_an_unknown_entry_is_kept) {
    auto character = parse("name=uta\n"
                           "genre=pop\n");

    BOOST_REQUIRE_EQUAL(character.extraLines.size(), 1);
    BOOST_CHECK_EQUAL(character.extraLines.at(0), "genre=pop");
    BOOST_CHECK_EQUAL(written(character), crlf("name=uta\ngenre=pop\n"));
}

// The bytes are in the encoding of the author's machine, and the library does not decode them.
BOOST_AUTO_TEST_CASE(test_bytes_are_carried_as_they_are) {
    // Ge Ping in GBK, which is not valid UTF-8.
    const std::string gbk = "\xb8\xf0\xc6\xbd";
    auto character = parse("author=" + gbk + "\n");

    BOOST_CHECK_EQUAL(character.author, gbk);
    BOOST_CHECK_EQUAL(written(character), crlf("author=" + gbk + "\n"));
}

// A plugin.txt from a real UTAU installation ends without a terminator, as do many of these files.
BOOST_AUTO_TEST_CASE(test_a_last_line_without_a_newline_is_read) {
    auto character = parse("name=uta\nauthor=someone");

    BOOST_CHECK_EQUAL(character.author, "someone");
}

BOOST_AUTO_TEST_CASE(test_crlf_reads_the_same_as_lf) {
    BOOST_CHECK_EQUAL(parse("name=uta\r\n").name, "uta");
    BOOST_CHECK_EQUAL(parse("Version:1.0\r\n").extraLines.at(0), "Version:1.0");
}

// An empty value means that the entry is unspecified, so writing it back would add an entry that
// did not exist.
BOOST_AUTO_TEST_CASE(test_an_empty_entry_is_not_written) {
    CharacterTxt character;
    character.name = "uta";

    BOOST_CHECK_EQUAL(written(character), crlf("name=uta\n"));
}

BOOST_AUTO_TEST_CASE(test_write_reads_back_the_same) {
    const std::string text = "name=uta\n"
                             "image=icon.bmp\n"
                             "sample=sample.wav\n"
                             "author=someone\n"
                             "web=http://example.com/\n"
                             "Version:1.0\n";

    BOOST_CHECK_EQUAL(written(parse(text)), crlf(text));
}

BOOST_AUTO_TEST_SUITE_END()
