#include <string>

#include <stdutau/pluginfile.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_pluginfile)

// Only a section named by ASCII digits is a selected note. The notes around the selection have
// names of their own, and a section of any other name is skipped.
BOOST_AUTO_TEST_CASE(test_only_a_section_of_digits_is_a_selected_note) {
    PluginInput reader;
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
        PluginInput reader;
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
    PluginInput reader;
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
    PluginInput reader;
    BOOST_REQUIRE(reader.read("[#PREV]\nLength=240\nLyric=p\n"
                              "[#0002]\nLength=480\nLyric=a\n"
                              "[#0003]\nLength=480\nLyric=b\n"));
    BOOST_CHECK_EQUAL(reader.startIndex, 2);

    PluginInput empty;
    BOOST_REQUIRE(empty.read("[#PREV]\nLength=240\nLyric=p\n"));
    BOOST_CHECK_EQUAL(empty.startIndex, 0);
}

// The result made from the input changes nothing: a bare header for each note, numbered from
// the start of the selection, and nothing around the selection.
BOOST_AUTO_TEST_CASE(test_the_result_of_an_input_changes_nothing) {
    PluginInput input;
    input.startIndex = 2;
    input.prevNote = NoteExt(60, 480, "p");
    input.notes = {NoteExt(60, 480, "a"), NoteExt(60, 480, "b")};
    const PluginResult result(input);
    BOOST_CHECK(!result.isCancelled());
    BOOST_CHECK_EQUAL(result.write(), "[#0002]\r\n[#0003]\r\n");
}

// The host writes the file as UTAU does: version 1.20, the settings of a plugin, the notes
// around the selection as complete notes, the selection numbered from its position, the
// read-only entries last, CRLF, and no end of track. The plugin reads it back.
BOOST_AUTO_TEST_CASE(test_the_input_as_utau_writes_it) {
    NoteExt prev(60, 480, "a");
    prev.preUttrRO = 8.5;
    prev.aliasRO = "a2";
    NoteExt selected(62, 480, "i");
    selected.userData["$probe"] = "x";
    selected.preUttrRO = 20;
    selected.overlapRO = 12.5;
    selected.stpRO = 0;
    selected.filenameRO = "i.wav";

    PluginInput writer;
    writer.settings.project = "C:\\song.ust";
    writer.settings.tempo = 150;
    writer.settings.voiceDir = "C:\\voice";
    writer.settings.cacheDir = "C:\\song.cache";
    writer.settings.isMode2 = true;
    writer.settings.projectName = "unused";
    writer.prevNote = prev;
    writer.startIndex = 2;
    writer.notes = {selected, NoteExt(60, 240, "R")};
    writer.nextNote = NoteExt(64, 480, "u");
    const auto text = writer.write();

    BOOST_CHECK_EQUAL(text.rfind("[#VERSION]\r\nUST Version 1.20\r\n"
                                 "[#SETTING]\r\nProject=C:\\song.ust\r\nTempo=150\r\n"
                                 "VoiceDir=C:\\voice\r\nCacheDir=C:\\song.cache\r\nMode2=True\r\n"
                                 "[#PREV]\r\n",
                                 0),
                      0);
    BOOST_CHECK(text.find("[#0002]\r\nLength=480\r\nLyric=i\r\nNoteNum=62\r\nPreUtterance=\r\n") !=
                std::string::npos);
    BOOST_CHECK(text.find("$probe=x\r\n@preuttr=20\r\n@overlap=12.5\r\n@stpoint=0\r\n"
                          "@filename=i.wav\r\n[#0003]\r\n") != std::string::npos);
    BOOST_CHECK(text.find("@alias=a2\r\n") != std::string::npos);
    BOOST_CHECK_EQUAL(text.find("ProjectName"), std::string::npos);
    BOOST_CHECK_EQUAL(text.find("TRACKEND"), std::string::npos);
    BOOST_CHECK_EQUAL(text.substr(text.size() - 2), "\r\n");

    PluginInput reader;
    BOOST_REQUIRE(reader.read(text));
    BOOST_CHECK_EQUAL(reader.settings.project, "C:\\song.ust");
    BOOST_CHECK_EQUAL(reader.settings.tempo, 150);
    BOOST_CHECK(reader.settings.isMode2);
    BOOST_CHECK_EQUAL(reader.startIndex, 2);
    BOOST_REQUIRE_EQUAL(reader.notes.size(), 2);
    BOOST_CHECK_EQUAL(reader.notes[0].filenameRO, "i.wav");
    BOOST_CHECK_EQUAL(reader.notes[0].userData.at("$probe"), "x");
    BOOST_CHECK_EQUAL(reader.notes[1].lyric, "R");
    BOOST_REQUIRE(reader.prevNote);
    BOOST_CHECK_EQUAL(reader.prevNote->aliasRO, "a2");
    BOOST_REQUIRE(reader.nextNote);
    BOOST_CHECK_EQUAL(reader.nextNote->lyric, "u");
}

// A plugin that receives the whole track gets no notes around the selection.
BOOST_AUTO_TEST_CASE(test_the_input_without_notes_around_the_selection) {
    PluginInput writer;
    writer.notes = {NoteExt(60, 480, "a")};
    const auto text = writer.write();
    BOOST_CHECK_EQUAL(text.find("[#PREV]"), std::string::npos);
    BOOST_CHECK_EQUAL(text.find("[#NEXT]"), std::string::npos);
    BOOST_CHECK(text.find("[#0000]") != std::string::npos);
}

// The sections apply in their order, whatever their numbers, and each records the entries it
// has. This is the result of the probe that UTAU applied in docs/claude/utau-plugin-protocol.md
// of HelloUtau.
BOOST_AUTO_TEST_CASE(test_the_result_in_its_order) {
    PluginResult reader;
    BOOST_REQUIRE(reader.read("[#PREV]\r\nLyric=P\r\n"
                              "[#INSERT]\r\nLyric=N\r\n"
                              "[#0002]\r\n"
                              "[#DELETE]\r\n"
                              "[#0004]\r\nVelocity=\r\nLength=240\r\n"
                              "[#NEXT]\r\nLyric=Q\r\n"));
    BOOST_CHECK(!reader.isCancelled());
    using Section = PluginResult::Section;
    const auto &sections = reader.sections;
    BOOST_REQUIRE_EQUAL(sections.size(), 6);

    BOOST_CHECK_EQUAL(sections[0].kind, Section::Prev);
    BOOST_CHECK(sections[0].keys == std::set<std::string>({"Lyric"}));
    BOOST_CHECK_EQUAL(sections[0].note.lyric, "P");

    BOOST_CHECK_EQUAL(sections[1].kind, Section::Insert);
    BOOST_CHECK_EQUAL(sections[1].note.lyric, "N");

    BOOST_CHECK_EQUAL(sections[2].kind, Section::Numbered);
    BOOST_CHECK_EQUAL(sections[2].number, 2);
    BOOST_CHECK(sections[2].keys.empty());

    BOOST_CHECK_EQUAL(sections[3].kind, Section::Delete);

    // An empty value is present and leaves its member blank.
    BOOST_CHECK_EQUAL(sections[4].number, 4);
    BOOST_CHECK(sections[4].keys == std::set<std::string>({"Length", "Velocity"}));
    BOOST_CHECK(!sections[4].note.velocity);
    BOOST_CHECK_EQUAL(sections[4].note.length, 240);

    BOOST_CHECK_EQUAL(sections[5].kind, Section::Next);
    BOOST_CHECK_EQUAL(sections[5].note.lyric, "Q");
}

// Entries read under several names are recorded under one. Read-only entries are left out. An
// empty value leaves even a member that a note otherwise fills, such as the vibrato, blank.
BOOST_AUTO_TEST_CASE(test_the_entries_of_a_result) {
    PluginResult reader;
    BOOST_REQUIRE(reader.read("[#0000]\nModuration=5\nPiches=1,2\nPBType=5\n@preuttr=3\n"
                              "@filename=a.wav\nVBR=\nEnvelope=\nLyric=\nIntensity=\n$user=\n"));
    BOOST_REQUIRE_EQUAL(reader.sections.size(), 1);
    const auto &section = reader.sections[0];
    BOOST_CHECK(section.keys == std::set<std::string>({"$user", "Envelope", "Intensity", "Lyric",
                                                       "Modulation", "PitchBend", "VBR"}));
    BOOST_CHECK_EQUAL(section.note.modulation.value_or(-1), 5);
    BOOST_CHECK_EQUAL(section.note.pitches.size(), 2);
    BOOST_CHECK(!section.note.vibrato);
    BOOST_CHECK(!section.note.envelope);
    BOOST_CHECK(!section.note.intensity);
    BOOST_CHECK(section.note.lyric.empty());
    BOOST_CHECK(section.note.userData.empty());
}

// A file without a section of a note cancels the plugin, even with its version and settings.
BOOST_AUTO_TEST_CASE(test_a_result_without_notes_cancels) {
    PluginResult empty;
    BOOST_REQUIRE(empty.read(""));
    BOOST_CHECK(empty.isCancelled());

    PluginResult settings;
    BOOST_REQUIRE(settings.read("[#VERSION]\r\nUST Version 1.20\r\n[#SETTING]\r\nTempo=120\r\n"
                                "[#TRACKEND]\r\n"));
    BOOST_CHECK(settings.isCancelled());
}

// Assigning a note records the entries it has, leaving out PreUtterance without a value, which
// would restore the default, and the entries that the file derives.
BOOST_AUTO_TEST_CASE(test_assigning_a_note) {
    Note note(65, 480, "z");
    note.modulation.reset();
    note.pitches = {1, 2};
    note.userData["$user"] = "x";
    PluginResult::Section section;
    section.assign(note);
    BOOST_CHECK(section.keys == std::set<std::string>({"$user", "Intensity", "Length", "Lyric",
                                                       "NoteNum", "PBStart", "PitchBend"}));
    BOOST_CHECK_EQUAL(section.note.lyric, "z");
}

// What a plugin writes, a host reads back as it was: the sections in their order, the entries
// in each, and an entry with an empty value, which restores the default.
BOOST_AUTO_TEST_CASE(test_the_result_that_a_plugin_writes) {
    using Section = PluginResult::Section;
    PluginInput input;
    input.startIndex = 2;
    input.notes = {NoteExt(60, 480, "a"), NoteExt(62, 480, "b"), NoteExt(64, 480, "c")};

    PluginResult result(input);
    Note changed = input.notes[0];
    changed.lyric = "z";
    changed.intensity.reset();
    result.sections[0].assign(changed);
    result.sections[0].keys.insert("Intensity");
    result.sections[1] = Section(Section::Delete);
    Section inserted(Section::Insert);
    inserted.assign(Note(70, 120, "n"));
    result.sections.insert(result.sections.begin() + 2, inserted);
    Section prev(Section::Prev);
    Note previous(60, 480, "p");
    previous.pitches = {1, 2};
    prev.assign(previous);
    result.sections.insert(result.sections.begin(), prev);

    const auto text = result.write();
    BOOST_CHECK(text.find("[#0002]\r\nLength=480\r\nLyric=z\r\n") != std::string::npos);
    BOOST_CHECK(text.find("Intensity=\r\n") != std::string::npos);
    BOOST_CHECK(text.find("[#DELETE]\r\n[#INSERT]\r\n") != std::string::npos);
    // The type of the Mode1 pitch goes with its values.
    BOOST_CHECK(text.find("PBType=5\r\nPBStart=0\r\nPitchBend=1,2\r\n") != std::string::npos);
    BOOST_CHECK(text.find("[#0004]\r\n") != std::string::npos);

    PluginResult back;
    BOOST_REQUIRE(back.read(text));
    BOOST_REQUIRE_EQUAL(back.sections.size(), result.sections.size());
    for (size_t i = 0; i < back.sections.size(); ++i) {
        BOOST_TEST_CONTEXT("section " << i) {
            BOOST_CHECK_EQUAL(back.sections[i].kind, result.sections[i].kind);
            BOOST_CHECK(back.sections[i].keys == result.sections[i].keys);
        }
    }
    BOOST_CHECK_EQUAL(back.sections[1].note.lyric, "z");
    BOOST_CHECK(!back.sections[1].note.intensity);
    BOOST_CHECK_EQUAL(back.sections[3].note.length, 120);
    BOOST_CHECK_EQUAL(back.sections[4].number, 4);
    BOOST_CHECK(back.sections[4].keys.empty());
}

BOOST_AUTO_TEST_SUITE_END()
