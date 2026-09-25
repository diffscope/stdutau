#include "pluginfile.h"

#include <fstream>
#include <algorithm>

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

    PluginFileReader::PluginFileReader() : startIndex(0) {
    }

    bool PluginFileReader::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool PluginFileReader::read(std::string_view text) {
        // Whether the file ends without a terminator, which determines when the loop below
        // reaches the last line. See UstFile::read, which parses the same structure.
        const bool dangling = !text.empty() && text.back() != '\n';

        // Read File
        std::vector<std::string> currentSection;

        std::string_view line;
        while (takeLine(text, line)) {
            const bool atEnd = text.empty() && dangling;

            if (line.empty() && !atEnd) {
                continue;
            }

            // Continue to add until meet the start of section or end
            if (!starts_with(line, SECTION_BEGIN_MARK) && !atEnd) {
                currentSection.emplace_back(line);
                continue;
            }

            // If meet end, append without continue
            if (!line.empty() && atEnd) {
                currentSection.emplace_back(line);
            }

            // Previous section is empty
            if (currentSection.size() <= 1) {
                // ...
            } else {
                const auto &sectionHead = currentSection[0];

                // If Section Name is invalid
                std::string_view sectionName;
                if (!parseSectionName(sectionHead, sectionName)) {
                    currentSection.clear();
                    continue;
                }

                if (sectionName == SECTION_NAME_VERSION) {
                    // Parse Version Sequence
                    parseSectionVersion(currentSection, version);
                } else if (sectionName == SECTION_NAME_SETTING) {
                    // Parse global settings
                    parseSectionSettings(currentSection, settings);
                } else if (std::all_of(sectionName.begin(), sectionName.end(), isAsciiDigit)) {
                    // Parse Note (Name should be numeric)
                    auto note = createInitialNoteExt();
                    parseSectionNoteExt(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        notes.push_back(note);
                    }
                } else if (sectionName == SECTION_NAME_PREV) {
                    auto note = createInitialNoteExt();
                    parseSectionNoteExt(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        prevNote = note;
                    }
                } else if (sectionName == SECTION_NAME_NEXT) {
                    auto note = createInitialNoteExt();
                    parseSectionNoteExt(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        nextNote = note;
                    }
                }
            }

            currentSection.clear();
            currentSection.emplace_back(line);
        }

        return true;
    }

    PluginFileWriter::PluginFileWriter(int startIndex, int originalSize)
        : m_startIndex(startIndex), m_originalSize(originalSize) {
    }

    bool PluginFileWriter::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    std::string PluginFileWriter::write() const {
        std::string out;

        struct NoteItem {
            bool removed = false;
            const Note *changed = nullptr;
            const std::vector<Note> *inserted = nullptr;
        };

        // Collect data
        std::vector<NoteItem> noteItems(m_originalSize + 1); // Reserved the last for insertion
        for (const auto &item : m_changedNotes) {
            noteItems[item.first - m_startIndex].changed = &item.second;
        }
        for (const auto &item : m_insertedNotes) {
            noteItems[item.first - m_startIndex].inserted = &item.second;
        }
        for (const auto &item : m_removedNotes) {
            noteItems[item - m_startIndex].removed = true;
        }

        // Previous
        if (!m_notesBeforePrev.empty()) {
            for (const auto &note : m_notesBeforePrev) {
                writeSectionName(SECTION_NAME_INSERT, out);
                writeSectionNote(-1, note, out);
            }

            // Complement
            if (!m_prevNote) {
                writeSectionName(SECTION_NAME_PREV, out);
            }
        }
        if (m_prevNote) {
            writeSectionName(SECTION_NAME_PREV, out);
            writeSectionNote(-1, m_prevNote.value(), out);
        }

        // Selection
        for (int i = 0; i < noteItems.size(); ++i) {
            const auto &item = noteItems[i];
            if (item.inserted) {
                for (const auto &note : *item.inserted) {
                    writeSectionName(SECTION_NAME_INSERT, out);
                    writeSectionNote(-1, note, out);
                }
            }

            // Skip last
            if (i + 1 == noteItems.size())
                break;

            if (item.removed) {
                writeSectionName(SECTION_NAME_DELETE, out);
                continue;
            }

            int idx = m_startIndex + i;
            if (item.changed) {
                writeSectionNote(idx, *item.changed, out);
                continue;
            }

            // Keep index
            writeSectionName(idx, out);
        }

        // Next
        if (m_nextNote) {
            writeSectionName(SECTION_NAME_NEXT, out);
            writeSectionNote(-1, m_nextNote.value(), out);
        }
        if (!m_notesAfterNext.empty()) {
            // Complement
            if (!m_nextNote) {
                writeSectionName(SECTION_NAME_NEXT, out);
            }

            for (const auto &note : m_notesAfterNext) {
                writeSectionName(SECTION_NAME_INSERT, out);
                writeSectionNote(-1, note, out);
            }
        }
        return out;
    }

    void PluginFileWriter::setNote(int index, const Note &note) {
        m_changedNotes[index] = note;
    }

    void PluginFileWriter::setPrevNote(const Note &note) {
        m_prevNote = note;
    }

    void PluginFileWriter::setNextNote(const Note &note) {
        m_nextNote = note;
    }

    void PluginFileWriter::insertNotes(int index, const std::vector<Note> &notes) {
        auto &vec = m_insertedNotes[index];
        vec.insert(vec.end(), notes.begin(), notes.end());
    }

    void PluginFileWriter::prependNotesBeforePrev(const std::vector<Note> &notes) {
        auto &vec = m_notesBeforePrev;
        vec.insert(vec.begin(), notes.begin(), notes.end());
    }

    void PluginFileWriter::appendNotesAfterNext(const std::vector<Note> &notes) {
        auto &vec = m_notesAfterNext;
        vec.insert(vec.end(), notes.begin(), notes.end());
    }

    void PluginFileWriter::removeNote(int index) {
        m_removedNotes.insert(index);
    }

    void PluginFileWriter::removeNotes(const std::vector<int> &indexes) {
        for (const auto &idx : indexes)
            m_removedNotes.insert(idx);
    }

}
