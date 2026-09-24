#include <string>

#include <stdutau/otoini.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_otoini)

namespace {

    OtoIni parse(const std::string &text) {
        OtoIni oto;
        BOOST_REQUIRE(oto.read(text));
        return oto;
    }

}

BOOST_AUTO_TEST_CASE(test_read) {
    auto oto = parse("a.wav=a,100,200,300,400,50\n");

    BOOST_REQUIRE_EQUAL(oto.contents.size(), 1);
    const auto &samples = oto.contents.at("a.wav");
    BOOST_REQUIRE_EQUAL(samples.size(), 1);
    BOOST_CHECK_EQUAL(samples.at(0).alias, "a");
    BOOST_CHECK_EQUAL(samples.at(0).offset, 100);
    BOOST_CHECK_EQUAL(samples.at(0).consonant, 200);
    BOOST_CHECK_EQUAL(samples.at(0).cutoff, 300);
    BOOST_CHECK_EQUAL(samples.at(0).preUtterance, 400);
    BOOST_CHECK_EQUAL(samples.at(0).voiceOverlap, 50);
}

// One sample file may have several aliases, which is the common case in a voice bank rather than
// an exception.
BOOST_AUTO_TEST_CASE(test_one_file_may_have_several_entries) {
    auto oto = parse("a.wav=a,0,0,0,0,0\n"
                     "a.wav=- a,0,0,0,0,0\n");

    BOOST_REQUIRE_EQUAL(oto.contents.size(), 1);
    BOOST_REQUIRE_EQUAL(oto.contents.at("a.wav").size(), 2);
    BOOST_CHECK_EQUAL(oto.contents.at("a.wav").at(1).alias, "- a");
}

// The CRLF line endings of a voice bank written on Windows. Text mode removes them only on
// Windows, so on other systems the last field of every line would end with a carriage return.
//
// The entry has no commas, which is the form that exposes the difference: the alias is the only
// text field, and a line that also contains the numbers ends with a number, where a trailing
// carriage return is discarded by the conversion and the test would prove nothing.
BOOST_AUTO_TEST_CASE(test_crlf_reads_the_same_as_lf) {
    BOOST_CHECK_EQUAL(parse("a.wav=myalias\r\n").contents.at("a.wav").at(0).alias, "myalias");

    auto crlf = parse("a.wav=a,100,200,300,400,50\r\n");
    BOOST_REQUIRE_EQUAL(crlf.contents.size(), 1);
    BOOST_CHECK_EQUAL(crlf.contents.at("a.wav").at(0).alias, "a");
    BOOST_CHECK_EQUAL(crlf.contents.at("a.wav").at(0).voiceOverlap, 50);
}

// Both forms on two lines of one sample, as in a real voice bank. A save that reformatted either
// would turn a one-line edit into a change of the entire file.
BOOST_AUTO_TEST_CASE(test_numbers_keep_their_spelling) {
    const std::string text = "a.wav=a,41,87.688,97.316,8.938,4.457\r\n"
                             "a.wav=a -,41.0,87.6880,-143.414,8.938,04.457\r\n";
    BOOST_CHECK_EQUAL(parse(text).write(), text);
}

BOOST_AUTO_TEST_CASE(test_a_changed_number_is_written_afresh) {
    auto oto = parse("a.wav=a,41.0,87.688,97.316,8.938,4.457\r\n");
    auto &entry = oto.contents.at("a.wav").at(0);
    entry.offset = 12345.678;
    entry.cutoff = -250;
    BOOST_CHECK_EQUAL(oto.write(), "a.wav=a,12345.678,87.688,-250,8.938,4.457\r\n");
}

// The declaration is not an entry, and a save that dropped it would make a program that honors it
// read the UTF-8 file in the code page of the machine.
BOOST_AUTO_TEST_CASE(test_the_charset_declaration_is_kept) {
    const std::string text = "#Charset:UTF-8\r\n"
                             "a.wav=a,41,87.688,97.316,8.938,4.457\r\n";
    auto oto = parse(text);
    BOOST_CHECK_EQUAL(oto.charset, "UTF-8");
    BOOST_CHECK_EQUAL(oto.contents.size(), 1);
    BOOST_CHECK_EQUAL(oto.write(), text);
}

// The declaration is recognized regardless of case and written in one form, before the entries
// even if it followed them. Only the first declaration is kept.
BOOST_AUTO_TEST_CASE(test_the_charset_declaration_is_written_first) {
    auto oto = parse("a.wav=a,0,0,0,0,0\r\n"
                     "#CHARSET:utf-8\r\n"
                     "#Charset:Shift_JIS\r\n");
    BOOST_CHECK_EQUAL(oto.charset, "utf-8");
    BOOST_CHECK_EQUAL(oto.write(), "#Charset:utf-8\r\n"
                                   "a.wav=a,0,0,0,0,0\r\n");
}

// An offset beyond ten seconds has more than six significant digits, the default stream precision.
BOOST_AUTO_TEST_CASE(test_an_entry_never_read_loses_no_digits) {
    OtoIni oto;
    OtoEntry entry;
    entry.fileName = "a.wav";
    entry.alias = "a";
    entry.offset = 123456.789;
    entry.voiceOverlap = 0.1;
    entry.preUtterance = 1e21;
    oto.contents["a.wav"].push_back(entry);
    BOOST_CHECK_EQUAL(oto.write(), "a.wav=a,123456.789,0,0,1000000000000000000000,0.1\r\n");
}

BOOST_AUTO_TEST_SUITE_END()
