#include <cstring>
#include <string>
#include <vector>

#include <stdutau/ustfile.h>
#include <stdutau/utautils.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_ustfile)

namespace {

    // The reader hands a section off when it meets the next header, so the trailing
    // [#TRACKEND] is what makes the last note arrive.
    std::string ust(const std::vector<std::string> &noteLines) {
        std::string s = "[#VERSION]\nUST Version1.2\n[#SETTING]\nTempo=120.00\nTracks=1\n[#0000]\n";
        for (const auto &line : noteLines) {
            s += line;
            s += '\n';
        }
        s += "[#TRACKEND]\n";
        return s;
    }

    UstFile parse(const std::string &text) {
        UstFile file;
        BOOST_REQUIRE(file.read(text));
        BOOST_REQUIRE_EQUAL(file.notes.size(), 1);
        return file;
    }

    std::string serialize(const UstFile &file) {
        return file.write();
    }

    int occurrences(const std::string &haystack, const std::string &needle) {
        int n = 0;
        for (auto pos = haystack.find(needle); pos != std::string::npos;
             pos = haystack.find(needle, pos + needle.size())) {
            n++;
        }
        return n;
    }

    // base64url without padding, the alphabet a host can put in a value and get back unchanged
    // from UTAU as well as from here.
    std::string payload(std::size_t n) {
        static const std::string alphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string s;
        s.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            s += alphabet[i % alphabet.size()];
        }
        return s;
    }

    const std::vector<std::string> minimalNote = {
        "Length=480",
        "Lyric=a",
        "NoteNum=60",
        "PreUtterance=",
    };

    std::vector<std::string> noteWith(const std::vector<std::string> &extra) {
        auto lines = minimalNote;
        lines.insert(lines.end(), extra.begin(), extra.end());
        return lines;
    }

}

BOOST_AUTO_TEST_CASE(test_unknown_entries_become_user_data) {
    auto file = parse(ust(noteWith({
        "$hup_charset=Shift_JIS",
        "WhateverElse=kept too",
    })));
    const auto &note = file.notes.at(0);

    BOOST_CHECK_EQUAL(note.userData.size(), 2);
    BOOST_CHECK_EQUAL(note.userData.at("$hup_charset"), "Shift_JIS");

    // A name without the leading $ is kept here although UTAU would drop it. What this library
    // reads back is its own business, and a host that wants the entry to survive UTAU is the one
    // that has to choose the name.
    BOOST_CHECK_EQUAL(note.userData.at("WhateverElse"), "kept too");
}

// An entry the file leaves out is absent, not zero, and the real* accessors are where the
// default belongs. Telling the two apart is the whole reason these are optional.
BOOST_AUTO_TEST_CASE(test_omitted_entries_stay_absent) {
    auto file = parse(ust(minimalNote));
    const auto &note = file.notes.at(0);

    BOOST_CHECK(!note.intensity);
    BOOST_CHECK(!note.modulation);
    BOOST_CHECK(!note.velocity);
    BOOST_CHECK(!note.overlap);
    BOOST_CHECK(!note.stp);
    BOOST_CHECK(!note.tempo);
    BOOST_CHECK(!note.pbstart);

    BOOST_CHECK_EQUAL(note.realIntensity(), DEFAULT_VALUE_INTENSITY);
    BOOST_CHECK_EQUAL(note.realModulation(), DEFAULT_VALUE_MODULATION);
    BOOST_CHECK_EQUAL(note.realVelocity(), DEFAULT_VALUE_VELOCITY);
    BOOST_CHECK_EQUAL(note.realStartPoint(), DEFAULT_VALUE_START_POINT);
}

// UST always carries PreUtterance, empty where the note has none. Present and empty has to read
// the same as missing.
BOOST_AUTO_TEST_CASE(test_entry_present_but_empty_stays_absent) {
    BOOST_CHECK(!parse(ust(minimalNote)).notes.at(0).preUttr);
    BOOST_CHECK(!parse(ust(noteWith({"Tempo="}))).notes.at(0).tempo);

    auto given = parse(ust(noteWith({"Tempo=0"}))).notes.at(0).tempo;
    BOOST_REQUIRE(given.has_value());
    BOOST_CHECK_EQUAL(*given, 0);
}

// What was absent has to come back absent, or a file gains values every time it is opened.
BOOST_AUTO_TEST_CASE(test_absence_survives_a_round_trip) {
    auto once = serialize(parse(ust(minimalNote)));

    UstFile second;
    BOOST_REQUIRE(second.read(once));
    BOOST_REQUIRE_EQUAL(second.notes.size(), 1);

    BOOST_CHECK(!second.notes.at(0).intensity);
    BOOST_CHECK(!second.notes.at(0).tempo);
    BOOST_CHECK_EQUAL(serialize(second), once);
}

BOOST_AUTO_TEST_CASE(test_known_entries_stay_out_of_user_data) {
    auto file = parse(ust(noteWith({
        "VoiceOverlap=5",
        "Velocity=120",
        "Intensity=80",
        "Modulation=30",
        "StartPoint=1.5",
        "Flags=g-5",
        "Tempo=130",
        "PBStart=-20",
        "PitchBend=0,10,20",
        "PBS=-40;0",
        "PBW=50",
        "PBY=10",
        "PBM=s",
        "VBR=65,180,35,20,20,0,0",
        "Envelope=0,5,35,0,100,100,0",
        "$region=verse",
        "$region_end=verse",
    })));

    BOOST_CHECK(file.notes.at(0).userData.empty());
}

// Moduration is how some files spell Modulation, and the reader takes both. Neither may land in
// userData, or writing the note back would emit the value twice under two names.
BOOST_AUTO_TEST_CASE(test_misspelled_modulation_stays_out_of_user_data) {
    auto file = parse(ust(noteWith({"Moduration=30"})));

    BOOST_CHECK(file.notes.at(0).userData.empty());
    BOOST_REQUIRE(file.notes.at(0).modulation.has_value());
    BOOST_CHECK_EQUAL(*file.notes.at(0).modulation, 30);
}

// These are either read under another name or worked out again on the way out. Keeping them as
// user data would write them a second time.
BOOST_AUTO_TEST_CASE(test_reserved_entries_stay_out_of_user_data) {
    auto file = parse(ust(noteWith({
        "PBType=5",
        "@preuttr=100",
        "@overlap=50",
        "@stpoint=10",
        "@filename=a.wav",
        "@alias=a",
        "@cache=a.wav.frq",
    })));

    BOOST_CHECK(file.notes.at(0).userData.empty());
}

BOOST_AUTO_TEST_CASE(test_pbtype_is_written_once) {
    auto file = parse(ust(noteWith({"PBType=5", "PitchBend=0,10,20"})));

    BOOST_CHECK_EQUAL(occurrences(serialize(file), "PBType="), 1);
}

BOOST_AUTO_TEST_CASE(test_label_direct_and_patch_have_their_own_fields) {
    auto file = parse(ust(noteWith({
        "Label=chorus",
        "$direct=True",
        "$patch=take3.wav",
    })));
    const auto &note = file.notes.at(0);

    BOOST_CHECK(note.userData.empty());
    BOOST_CHECK_EQUAL(note.label, "chorus");
    BOOST_CHECK_EQUAL(note.direct, "True");
    BOOST_CHECK_EQUAL(note.patch, "take3.wav");

    auto text = serialize(file);
    BOOST_CHECK_EQUAL(occurrences(text, "Label=chorus"), 1);
    BOOST_CHECK_EQUAL(occurrences(text, "$direct=True"), 1);
    BOOST_CHECK_EQUAL(occurrences(text, "$patch=take3.wav"), 1);
}

// UTAU resolves a repeated entry to the last one it read. So does this.
BOOST_AUTO_TEST_CASE(test_repeated_entry_keeps_the_last) {
    auto file = parse(ust(noteWith({"$hup_data=first", "$hup_data=second"})));

    BOOST_CHECK_EQUAL(file.notes.at(0).userData.size(), 1);
    BOOST_CHECK_EQUAL(file.notes.at(0).userData.at("$hup_data"), "second");
}

BOOST_AUTO_TEST_CASE(test_user_data_is_written_at_the_end_of_the_section) {
    auto file = parse(ust(noteWith({"$hup_data=AAAA"})));
    auto text = serialize(file);

    auto entry = text.find("$hup_data=AAAA");
    BOOST_REQUIRE(entry != std::string::npos);

    // Everything the note knows about itself comes first, and the section ends right after.
    BOOST_CHECK(entry > text.find("NoteNum="));
    BOOST_CHECK_EQUAL(text.find("[#TRACKEND]"),
                      entry + std::string("$hup_data=AAAA").size() + std::strlen(LINE_END));
}

BOOST_AUTO_TEST_CASE(test_round_trip_keeps_user_data) {
    auto once = serialize(parse(ust(noteWith({
        "$hup_charset=Shift_JIS",
        "$hup_plugins=abc-_123",
        "Label=chorus",
    }))));

    // Reading what was written and writing it again has to land on the same bytes. A value that
    // survives one pass and not the next is the failure this is watching for.
    UstFile second;
    BOOST_REQUIRE(second.read(once));
    BOOST_CHECK_EQUAL(serialize(second), once);

    BOOST_REQUIRE_EQUAL(second.notes.size(), 1);
    BOOST_CHECK_EQUAL(second.notes.at(0).userData.at("$hup_charset"), "Shift_JIS");
    BOOST_CHECK_EQUAL(second.notes.at(0).label, "chorus");
}

// UTAU itself carries a value of this size without touching it, so the limit here must not be
// the lower one.
BOOST_AUTO_TEST_CASE(test_round_trip_keeps_a_long_value) {
    auto value = payload(65536);
    auto file = parse(ust(noteWith({"$hup_blob=" + value})));

    BOOST_CHECK_EQUAL(file.notes.at(0).userData.at("$hup_blob"), value);

    UstFile second;
    BOOST_REQUIRE(second.read(serialize(file)));
    BOOST_REQUIRE_EQUAL(second.notes.size(), 1);
    BOOST_CHECK_EQUAL(second.notes.at(0).userData.at("$hup_blob"), value);
}

// An entry whose value is empty still has a name, and dropping it would leave a host unable to
// tell "not set" from "set to nothing". UTAU does drop it, which is why a host cannot rely on
// the distinction reaching UTAU, but this library is not where it goes missing.
BOOST_AUTO_TEST_CASE(test_empty_value_is_kept) {
    auto file = parse(ust(noteWith({"$hup_empty="})));

    BOOST_REQUIRE_EQUAL(file.notes.at(0).userData.count("$hup_empty"), 1);
    BOOST_CHECK(file.notes.at(0).userData.at("$hup_empty").empty());

    UstFile second;
    BOOST_REQUIRE(second.read(serialize(file)));
    BOOST_REQUIRE_EQUAL(second.notes.size(), 1);
    BOOST_CHECK_EQUAL(second.notes.at(0).userData.count("$hup_empty"), 1);
}

// UTAU writes CRLF, and this library reads bytes, so the carriage return arrives on every
// platform. Without takeLine() this content parsed to nothing at all: the section header ended
// in a carriage return, which is not the closing bracket the reader looks for, so every section
// was thrown away.
BOOST_AUTO_TEST_CASE(test_crlf_reads_the_same_as_lf) {
    auto text = ust(noteWith({"Label=chorus", "$hup_data=AAAA"}));
    std::string crlf;
    for (char c : text) {
        if (c == '\n') {
            crlf += '\r';
        }
        crlf += c;
    }

    UstFile file;
    BOOST_REQUIRE(file.read(crlf));
    BOOST_REQUIRE_EQUAL(file.notes.size(), 1);

    const auto &note = file.notes.at(0);
    BOOST_CHECK_EQUAL(note.lyric, "a");
    BOOST_CHECK_EQUAL(note.label, "chorus");
    BOOST_CHECK_EQUAL(note.userData.at("$hup_data"), "AAAA");

    BOOST_CHECK_EQUAL(serialize(file), serialize(parse(text)));
}

BOOST_AUTO_TEST_SUITE_END()
