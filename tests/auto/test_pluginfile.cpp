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
                              "[#NEXT]\nLength=240\nLyric=n\n"));
    BOOST_REQUIRE_EQUAL(reader.notes.size(), 1);
    BOOST_CHECK_EQUAL(reader.notes.at(0).lyric, "a");
    BOOST_REQUIRE(reader.prevNote);
    BOOST_CHECK_EQUAL(reader.prevNote->lyric, "p");
    BOOST_REQUIRE(reader.nextNote);
    BOOST_CHECK_EQUAL(reader.nextNote->lyric, "n");
}

// The last section ends with the file, with or without a terminator. A header that names no
// section does not take the following section with it.
BOOST_AUTO_TEST_CASE(test_the_last_section_ends_with_the_file) {
    for (const char *end : {"", "\n", "\r\n"}) {
        PluginFileReader reader;
        BOOST_REQUIRE(reader.read(std::string("[#0000]\nLength=480\nLyric=a\n"
                                              "[#0001\nLength=480\nLyric=b\n"
                                              "[#NEXT]\nLength=240\nLyric=n") +
                                  end));
        BOOST_REQUIRE_EQUAL(reader.notes.size(), 1);
        BOOST_REQUIRE(reader.nextNote);
        BOOST_CHECK_EQUAL(reader.nextNote->lyric, "n");
    }
}

// An empty line is not an entry, and a note without a valid length is not a note, whether selected
// or around the selection.
BOOST_AUTO_TEST_CASE(test_empty_lines_and_notes_without_length_are_skipped) {
    PluginFileReader reader;
    BOOST_REQUIRE(reader.read("[#PREV]\nLength=-1\nLyric=p\n"
                              "[#0000]\nLength=480\n\nLyric=a\n"
                              "[#0001]\nLength=0\nLyric=b\n"
                              "[#NEXT]\nLength=0\nLyric=n\n"));
    BOOST_REQUIRE_EQUAL(reader.notes.size(), 1);
    BOOST_CHECK_EQUAL(reader.notes.at(0).lyric, "a");
    BOOST_CHECK(reader.notes.at(0).userData.empty());
    BOOST_CHECK(!reader.prevNote);
    BOOST_CHECK(!reader.nextNote);
}

BOOST_AUTO_TEST_SUITE_END()
