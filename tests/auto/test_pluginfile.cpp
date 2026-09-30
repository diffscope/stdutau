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

// UTAU numbers the selected notes by their positions in the track, so the first number is the
// position of the selection. A file without a selected note leaves it at zero.
BOOST_AUTO_TEST_CASE(test_the_first_number_is_the_start_of_the_selection) {
    PluginFileReader reader;
    BOOST_REQUIRE(reader.read("[#PREV]\nLength=240\nLyric=p\n"
                              "[#0002]\nLength=480\nLyric=a\n"
                              "[#0003]\nLength=480\nLyric=b\n"));
    BOOST_CHECK_EQUAL(reader.startIndex, 2);

    PluginFileReader empty;
    BOOST_REQUIRE(empty.read("[#PREV]\nLength=240\nLyric=p\n"));
    BOOST_CHECK_EQUAL(empty.startIndex, 0);
}

// Insertions stay within the selection: one past the last selected note inserts after it, before
// the next note, and the notes around the selection are written only when they are edited.
BOOST_AUTO_TEST_CASE(test_insertions_at_both_ends_of_the_selection) {
    Note note;
    note.length = 120;
    note.lyric = "x";

    PluginFileWriter writer(2, 2);
    writer.insertNotes(2, {note});
    writer.insertNotes(4, {note});
    const auto text = writer.write();
    BOOST_CHECK_EQUAL(text.find("[#PREV]"), std::string::npos);
    BOOST_CHECK_EQUAL(text.find("[#NEXT]"), std::string::npos);
    const auto first = text.find("[#INSERT]");
    const auto last = text.rfind("[#INSERT]");
    BOOST_REQUIRE_NE(first, last);
    BOOST_CHECK_LT(first, text.find("[#0002]"));
    BOOST_CHECK_GT(last, text.find("[#0003]"));
}

BOOST_AUTO_TEST_SUITE_END()
