#include "pluginfile.h"

#include <fstream>
#include <algorithm>
#include <charconv>

#include "private/usthelper_p.h"
#include "utautils.h"

namespace utau {

    inline NoteExt createInitialNoteExt() {
        NoteExt note;

        // The constructor initializes these with the values of a new note in an editor. A note
        // read from a file contains only the values the file specifies.
        note.intensity.reset();
        note.modulation.reset();

        return note;
    }

    PluginInput::PluginInput() : startIndex(0) {
    }

    bool PluginInput::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool PluginInput::read(std::string_view text) {
        // The lines of the current section, its header first. Lines before the first header
        // belong to no section. See UstFile::read, which parses the same structure.
        std::vector<std::string> currentSection;

        // Whether a numbered section has been read, which sets startIndex.
        bool numbered = false;

        // Parses the current section, if any. A section whose header names no section is
        // skipped.
        const auto parseSection = [&]() {
            if (currentSection.empty()) {
                return;
            }
            std::string_view sectionName;
            if (!parseSectionName(currentSection[0], sectionName)) {
                return;
            }
            if (sectionName == SECTION_NAME_VERSION) {
                parseSectionVersion(currentSection, version);
            } else if (sectionName == SECTION_NAME_SETTING) {
                parseSectionSettings(currentSection, settings);
            } else if (std::all_of(sectionName.begin(), sectionName.end(), isAsciiDigit) ||
                       sectionName == SECTION_NAME_PREV || sectionName == SECTION_NAME_NEXT) {
                // A selected note, whose section is named by its number, or a note around the
                // selection.
                if (!numbered && sectionName != SECTION_NAME_PREV &&
                    sectionName != SECTION_NAME_NEXT) {
                    // The first number is the position of the selection in the track.
                    numbered = true;
                    std::from_chars(sectionName.data(), sectionName.data() + sectionName.size(),
                                    startIndex);
                }
                auto note = createInitialNoteExt();
                parseSectionNoteExt(currentSection, note);
                // A note without a valid length is ignored.
                if (note.length <= 0) {
                    return;
                }
                if (sectionName == SECTION_NAME_PREV) {
                    prevNote = note;
                } else if (sectionName == SECTION_NAME_NEXT) {
                    nextNote = note;
                } else {
                    notes.push_back(note);
                }
            }
        };

        std::string_view line;
        while (takeLine(text, line)) {
            if (starts_with(line, SECTION_BEGIN_MARK)) {
                parseSection();
                currentSection.clear();
            }
            currentSection.emplace_back(line);
        }
        // The last section ends with the file, whether or not a terminator follows it.
        parseSection();
        return true;
    }

    bool PluginInput::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    std::string PluginInput::write() const {
        std::string out;

        writeSectionName(SECTION_NAME_VERSION, out);
        out += UST_VERSION_PREFIX;
        out += UST_VERSION_1_20;
        out += LINE_END;

        writeSectionName(SECTION_NAME_SETTING, out);
        writeEntry(out, KEY_NAME_PROJECT, settings.project);
        writeEntry(out, KEY_NAME_TEMPO, to_string(settings.tempo));
        writeEntry(out, KEY_NAME_VOICE_DIR, settings.voiceDir);
        writeEntry(out, KEY_NAME_CACHE_DIR, settings.cacheDir);
        if (settings.isMode2) {
            writeEntry(out, KEY_NAME_MODE2, VALUE_MODE2_ON);
        }

        if (prevNote) {
            writeSectionName(SECTION_NAME_PREV, out);
            writeSectionNoteExt(-1, *prevNote, out);
        }
        for (size_t i = 0; i < notes.size(); ++i) {
            writeSectionNoteExt(startIndex + int(i), notes[i], out);
        }
        if (nextNote) {
            writeSectionName(SECTION_NAME_NEXT, out);
            writeSectionNoteExt(-1, *nextNote, out);
        }
        return out;
    }

    // The name under which parseSectionNote() reads an entry, for the entries it reads under
    // several names.
    static std::string_view canonicalKey(std::string_view key) {
        if (key == KEY_NAME_MODURATION) {
            return KEY_NAME_MODULATION;
        }
        if (key == KEY_NAME_PICHES || key == KEY_NAME_PITCHES) {
            return KEY_NAME_PITCH_BEND;
        }
        return key;
    }

    PluginResult::Section::Section(Kind kind, int number) : kind(kind), number(number) {
        // A blank note, so that an entry with an empty value, which read() does not pass to
        // parseSectionNote(), leaves its member blank, and write() writes a blank member as an
        // empty value.
        note.lyric.clear();
        note.noteNum = 0;
        note.length = 0;
        note.intensity.reset();
        note.modulation.reset();
        note.pbtype.clear();
    }

    void PluginResult::Section::assign(const Note &note) {
        this->note = note;
        keys.clear();

        // The entries that writeSectionNote() writes for the note. It writes PreUtterance
        // even without a value, which would restore the default here.
        std::string text;
        writeSectionNote(-1, note, text);
        std::string_view rest = text;
        std::string_view line;
        while (takeLine(rest, line)) {
            const auto key = line.substr(0, line.find('='));
            if (isReservedKey(key) || (key == KEY_NAME_PRE_UTTERANCE && !note.preUttr)) {
                continue;
            }
            keys.emplace(key);
        }
    }

    PluginResult::PluginResult() = default;

    PluginResult::PluginResult(const PluginInput &input) {
        sections.reserve(input.notes.size());
        for (size_t i = 0; i < input.notes.size(); ++i) {
            sections.emplace_back(Section::Numbered, input.startIndex + int(i));
        }
    }

    bool PluginResult::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool PluginResult::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    bool PluginResult::read(std::string_view text) {
        // The lines of the current section, its header first. Lines before the first header
        // belong to no section.
        std::vector<std::string> currentSection;

        const auto parseSection = [&]() {
            if (currentSection.empty()) {
                return;
            }
            std::string_view sectionName;
            if (!parseSectionName(currentSection[0], sectionName)) {
                return;
            }

            Section section;
            if (!sectionName.empty() &&
                std::all_of(sectionName.begin(), sectionName.end(), isAsciiDigit)) {
                section.kind = Section::Numbered;
                std::from_chars(sectionName.data(), sectionName.data() + sectionName.size(),
                                section.number);
            } else if (sectionName == SECTION_NAME_INSERT) {
                section.kind = Section::Insert;
            } else if (sectionName == SECTION_NAME_DELETE) {
                section.kind = Section::Delete;
                sections.push_back(std::move(section));
                return;
            } else if (sectionName == SECTION_NAME_PREV) {
                section.kind = Section::Prev;
            } else if (sectionName == SECTION_NAME_NEXT) {
                section.kind = Section::Next;
            } else {
                return;
            }

            std::vector<std::string> valued;
            for (size_t i = 1; i < currentSection.size(); ++i) {
                const std::string_view line = currentSection[i];
                const auto eq = line.find('=');
                if (eq == std::string_view::npos) {
                    continue;
                }
                const auto key = line.substr(0, eq);
                if (isReservedKey(key)) {
                    continue;
                }
                section.keys.emplace(canonicalKey(key));
                if (eq + 1 < line.size()) {
                    valued.push_back(currentSection[i]);
                }
            }
            parseSectionNote(valued, section.note);
            sections.push_back(std::move(section));
        };

        std::string_view line;
        while (takeLine(text, line)) {
            if (starts_with(line, SECTION_BEGIN_MARK)) {
                parseSection();
                currentSection.clear();
            }
            currentSection.emplace_back(line);
        }
        // The last section ends with the file, whether or not a terminator follows it.
        parseSection();
        return true;
    }

    std::string PluginResult::write() const {
        std::string out;
        for (const auto &section : sections) {
            switch (section.kind) {
                case Section::Numbered:
                    writeSectionName(section.number, out);
                    break;
                case Section::Insert:
                    writeSectionName(SECTION_NAME_INSERT, out);
                    break;
                case Section::Delete:
                    writeSectionName(SECTION_NAME_DELETE, out);
                    continue;
                case Section::Prev:
                    writeSectionName(SECTION_NAME_PREV, out);
                    break;
                case Section::Next:
                    writeSectionName(SECTION_NAME_NEXT, out);
                    break;
            }
            if (section.keys.empty()) {
                continue;
            }

            // The entries in keys, as writeSectionNote() writes them from the note, with the
            // type of the Mode1 pitch along with its values. A key without a written entry has
            // a blank member and is written with an empty value.
            std::string text;
            writeSectionNote(-1, section.note, text);
            std::set<std::string_view> written;
            std::string_view rest = text;
            std::string_view line;
            while (takeLine(rest, line)) {
                const auto key = line.substr(0, line.find('='));
                const bool pitchType = key == KEY_NAME_PB_TYPE &&
                                       section.keys.count(KEY_NAME_PITCH_BEND) != 0;
                if (!pitchType && section.keys.count(std::string(key)) == 0) {
                    continue;
                }
                written.insert(key);
                out += line;
                out += LINE_END;
            }
            for (const auto &key : section.keys) {
                if (written.count(key) == 0) {
                    writeEntry(out, key, {});
                }
            }
        }
        return out;
    }

    bool PluginResult::isCancelled() const {
        return sections.empty();
    }

}
