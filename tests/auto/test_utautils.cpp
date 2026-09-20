#include <string>

#include <stdutau/utautils.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_utautils)

// An empty string and a string of nothing but spaces used to walk the iterator off the front.
BOOST_AUTO_TEST_CASE(test_trim) {
    BOOST_CHECK_EQUAL(trim(""), "");
    BOOST_CHECK_EQUAL(trim(" "), "");
    BOOST_CHECK_EQUAL(trim("   "), "");
    BOOST_CHECK_EQUAL(trim("a"), "a");
    BOOST_CHECK_EQUAL(trim("  a  "), "a");
    BOOST_CHECK_EQUAL(trim("a b"), "a b");

    // A byte above 0x7F is not a space, and asking whether it is must not be undefined. Raw
    // Shift_JIS reaches this function, so the question comes up in earnest.
    BOOST_CHECK_EQUAL(trim("\x82\xA0"), "\x82\xA0");
}

BOOST_AUTO_TEST_CASE(test_isRestLyric) {
    BOOST_CHECK(isRestLyric("R"));
    BOOST_CHECK(isRestLyric("r"));
    BOOST_CHECK(isRestLyric(""));
    BOOST_CHECK(isRestLyric("   "));
    BOOST_CHECK(isRestLyric("  R  "));
    BOOST_CHECK(!isRestLyric("a"));
    BOOST_CHECK(!isRestLyric("Ra"));
}

BOOST_AUTO_TEST_SUITE_END()
