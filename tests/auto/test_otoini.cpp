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

// One sample file may carry several aliases, which is the ordinary shape of a voice bank rather
// than an oddity.
BOOST_AUTO_TEST_CASE(test_one_file_may_have_several_entries) {
    auto oto = parse("a.wav=a,0,0,0,0,0\n"
                     "a.wav=- a,0,0,0,0,0\n");

    BOOST_REQUIRE_EQUAL(oto.contents.size(), 1);
    BOOST_REQUIRE_EQUAL(oto.contents.at("a.wav").size(), 2);
    BOOST_CHECK_EQUAL(oto.contents.at("a.wav").at(1).alias, "- a");
}

// The CRLF a voice bank written on Windows carries. Text mode strips it there and nowhere else,
// so the last field of every line arrived with a carriage return on it.
//
// The entry here has no commas, which is the shape that shows the difference: the alias is the
// only field holding text, and any line that gives the numbers as well ends on one of those,
// where a trailing carriage return is swallowed by the conversion and proves nothing.
BOOST_AUTO_TEST_CASE(test_crlf_reads_the_same_as_lf) {
    BOOST_CHECK_EQUAL(parse("a.wav=myalias\r\n").contents.at("a.wav").at(0).alias, "myalias");

    auto crlf = parse("a.wav=a,100,200,300,400,50\r\n");
    BOOST_REQUIRE_EQUAL(crlf.contents.size(), 1);
    BOOST_CHECK_EQUAL(crlf.contents.at("a.wav").at(0).alias, "a");
    BOOST_CHECK_EQUAL(crlf.contents.at("a.wav").at(0).voiceOverlap, 50);
}

// Both spellings on two lines of one sample, which is how a real bank has them. A save that
// respelled either would turn a one line edit into a diff of the whole file.
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

// Six significant digits was what a stream gave, and an offset past ten seconds has more.
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
