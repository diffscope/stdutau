#include <string>

#include <stdutau/pluginfile.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_pluginfile)

// Only a section named by ASCII digits is a selected note. The notes around the selection have
// names of their own, and a section of any other name is skipped.
BOOST_AUTO_TEST_CASE(test_only_a_section_of_digits_is_a_selected_note) {
    PluginFileReader reader;
    BOOST_REQUIRE(reader.read("[#SETTING]\nTempo=120.00\n"
                              "[#PREV]\nLength=240\nLyric=p\n"
                              "[#0000]\nLength=480\nLyric=a\n"
                              "[#INSERT]\nLength=480\nLyric=b\n"
                              "[#NEXT]\nLength=240\nLyric=n"));
    BOOST_REQUIRE_EQUAL(reader.notes.size(), 1);
    BOOST_CHECK_EQUAL(reader.notes.at(0).lyric, "a");
    BOOST_REQUIRE(reader.prevNote);
    BOOST_CHECK_EQUAL(reader.prevNote->lyric, "p");
    BOOST_REQUIRE(reader.nextNote);
    BOOST_CHECK_EQUAL(reader.nextNote->lyric, "n");
}

BOOST_AUTO_TEST_SUITE_END()
