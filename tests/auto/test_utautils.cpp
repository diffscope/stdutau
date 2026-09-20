#include <string>

#include <stdutau/utautils.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_utautils)

BOOST_AUTO_TEST_CASE(test_toDouble) {
    BOOST_CHECK_EQUAL(toDouble("1.5").value(), 1.5);
    BOOST_CHECK_EQUAL(toDouble("-2").value(), -2);
    BOOST_CHECK_EQUAL(toDouble("0").value(), 0);

    // Trailing rubbish still leaves a number in front of it.
    BOOST_CHECK_EQUAL(toDouble("1.5abc").value(), 1.5);

    BOOST_CHECK(!toDouble(""));
    BOOST_CHECK(!toDouble("abc"));

    // Out of range is no answer rather than an infinity. The library is built without exceptions,
    // so this has to come back as an empty optional and not as a throw.
    BOOST_CHECK(!toDouble("1e400"));

    // Neither a leading space nor a leading plus is accepted. The two parsing paths this library
    // has disagreed about both until they were made to answer alike.
    BOOST_CHECK(!toDouble(" 1.5"));
    BOOST_CHECK(!toDouble("+1.5"));
}

BOOST_AUTO_TEST_CASE(test_toInt) {
    BOOST_CHECK_EQUAL(toInt("42").value(), 42);
    BOOST_CHECK_EQUAL(toInt("-7").value(), -7);
    BOOST_CHECK_EQUAL(toInt("12abc").value(), 12);

    BOOST_CHECK(!toInt(""));
    BOOST_CHECK(!toInt("abc"));
    BOOST_CHECK(!toInt(" 42"));
    BOOST_CHECK(!toInt("+42"));
}

// stod2() and stoi2() answer the same for text that is a number, and hand back what they were
// given for text that is not. That is the whole difference from toDouble() and toInt().
BOOST_AUTO_TEST_CASE(test_stod2_and_stoi2_fall_back) {
    BOOST_CHECK_EQUAL(stod2("1.5", 99), 1.5);
    BOOST_CHECK_EQUAL(stod2("abc", 99), 99);
    BOOST_CHECK_EQUAL(stod2("", 99), 99);
    BOOST_CHECK_EQUAL(stod2("1e400", 99), 99);

    BOOST_CHECK_EQUAL(stoi2("42", 99), 42);
    BOOST_CHECK_EQUAL(stoi2("abc", 99), 99);
}

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

BOOST_AUTO_TEST_CASE(test_tone_names) {
    BOOST_CHECK_EQUAL(toneNameToToneNum("C1"), 24);
    BOOST_CHECK_EQUAL(toneNumToToneName(24), "C1");

    // 24 is C1, so middle C lands on 60 as it does in MIDI.
    BOOST_CHECK_EQUAL(toneNumToToneName(60), "C4");

    for (int num = 24; num <= 24 + 7 * 12 - 1; ++num) {
        BOOST_CHECK_EQUAL(toneNameToToneNum(toneNumToToneName(num)), num);
    }
}

// The name index reaches a subscript, so a caller handing over something out of range has to get
// an answer rather than undefined behavior. A negative key arrives here on its own, since the
// remainder of a negative number is negative in C++.
BOOST_AUTO_TEST_CASE(test_toneNumToToneName_refuses_an_index_out_of_range) {
    BOOST_CHECK_EQUAL(toneNumToToneName(-1, 0), "");
    BOOST_CHECK_EQUAL(toneNumToToneName(12, 0), "");
    BOOST_CHECK_EQUAL(toneNumToToneName(1000, 0), "");

    BOOST_CHECK_EQUAL(toneNumToToneName(-1), "");
    BOOST_CHECK_EQUAL(toneNumToToneName(0, 0), "C1");
}

BOOST_AUTO_TEST_CASE(test_split_and_join) {
    auto parts = split("a,b,,c", ",");
    BOOST_REQUIRE_EQUAL(parts.size(), 4);
    BOOST_CHECK_EQUAL(std::string(parts.at(2)), "");
    BOOST_CHECK_EQUAL(join(parts, ","), "a,b,,c");

    // Splitting text without the delimiter gives the text back, not nothing.
    BOOST_REQUIRE_EQUAL(split("abc", ",").size(), 1);
}

// A zero goes out empty and the empty ones at the end are dropped, which is how a UST keeps a
// pitch curve short.
BOOST_AUTO_TEST_CASE(test_doublesToStrings_drops_trailing_zeros) {
    auto strs = doublesToStrings({1, 0, 2, 0, 0});
    BOOST_REQUIRE_EQUAL(strs.size(), 3);
    BOOST_CHECK_EQUAL(strs.at(0), "1");
    BOOST_CHECK_EQUAL(strs.at(1), "");
    BOOST_CHECK_EQUAL(strs.at(2), "2");

    BOOST_CHECK(doublesToStrings({0, 0}).empty());
}

BOOST_AUTO_TEST_SUITE_END()
