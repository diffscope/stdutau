#include <sstream>
#include <string>

#include <stdutau/otoini.h>

#include <boost/test/unit_test.hpp>

using namespace Utau;

BOOST_AUTO_TEST_SUITE(test_otoini)

namespace {

    OtoIni parse(const std::string &text) {
        OtoIni oto;
        std::istringstream is(text);
        BOOST_REQUIRE(oto.read(is));
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

BOOST_AUTO_TEST_SUITE_END()
