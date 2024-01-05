#include "pluginfile.h"

#include <map>
#include <set>
#include <fstream>

#include "private/usthelper_p.h"
#include "utautils.h"

namespace Utau {

    inline NoteExt createInitialNoteExt() {
        NoteExt note;

        // These properties have explicit default values when created by editor
        // We need to reset them when reading the file
        note.intensity = NODEF_DOUBLE;
        note.modulation = NODEF_DOUBLE;

        return note;
    }

    /*!
        \class PluginFileReader
        \brief Plugin temporary text file reader.

        Use this class to read the \c .tmp file generated by UTAU when your plugin starts.
        Note that the \c Version and \c settings sections are readonly and not needed in the
        return file.
    */

    struct PluginFileReader::Private {
        UstVersion version;
        UstSettings settings;

        std::optional<NoteExt> prevNote;
        std::optional<NoteExt> nextNote;

        int startIndex = 0;
        std::vector<NoteExt> notes;
    };

    /*!
        Constructor.
    */
    PluginFileReader::PluginFileReader() : d_ptr(std::make_unique<Private>()) {
    }

    /*!
        Reads plugin information from stream, returns \c true if success.
    */
    bool PluginFileReader::load(const std::filesystem::path &path) {
        std::ifstream fs(path);
        if (!fs.is_open())
            return false;
        std::istream &is = fs;

        // Read File
        std::vector<std::string> currentSection;

        std::string line;
        while (std::getline(is, line)) {
            if (line.empty() && !is.eof()) {
                continue;
            }

            // Continue to add until meet the start of section or end
            if (!starts_with(line, SECTION_BEGIN_MARK) && !is.eof()) {
                currentSection.push_back(line);
                continue;
            }

            // If meet end, append without continue
            if (!line.empty() && is.eof()) {
                currentSection.push_back(line);
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
                    parseSectionVersion(currentSection, d_ptr->version);
                } else if (sectionName == SECTION_NAME_SETTING) {
                    // Parse global settings
                    parseSectionSettings(currentSection, d_ptr->settings);
                } else if (std::all_of(sectionName.begin(), sectionName.end(), ::isdigit)) {
                    // Parse Note (Name should be numeric)
                    auto note = createInitialNoteExt();
                    parseSectionNoteExt(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        d_ptr->notes.push_back(note);
                    }
                } else if (sectionName == SECTION_NAME_PREV) {
                    auto note = createInitialNoteExt();
                    parseSectionNoteExt(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        d_ptr->prevNote = note;
                    }
                } else if (sectionName == SECTION_NAME_NEXT) {
                    auto note = createInitialNoteExt();
                    parseSectionNoteExt(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        d_ptr->nextNote = note;
                    }
                }
            }

            currentSection.clear();
            currentSection.push_back(line);
        }
        return true;
    }

    UstVersion PluginFileReader::version() const {
        return d_ptr->version;
    }

    UstSettings PluginFileReader::settings() const {
        return d_ptr->settings;
    }

    std::optional<NoteExt> PluginFileReader::prevNote() const {
        return d_ptr->prevNote;
    }

    std::optional<NoteExt> PluginFileReader::nextNote() const {
        return d_ptr->nextNote;
    }

    int PluginFileReader::startIndex() const {
        return d_ptr->startIndex;
    }

    std::vector<NoteExt> PluginFileReader::notes() const {
        return d_ptr->notes;
    }

    /*!
        \class PluginFileWriter
        \brief Plugin temporary text file writer.

        Use this class to write the \c .tmp file as the return to UTAU when your plugin quits.
    */

    struct PluginFileWriter::Private {
        int startIndex = 0;
        int originalSize = 0;

        std::optional<Note> prevNote;
        std::optional<Note> nextNote;

        std::map<int, Note> changedNotes;
        std::map<int, std::vector<Note>> insertedNotes;
        std::set<int> removedNotes;

        std::vector<Note> notesBeforePrev;
        std::vector<Note> notesAfterNext;
    };

    /*!
        Constructor.
    */
    PluginFileWriter::PluginFileWriter(int startIndex, int originalSize)
        : d_ptr(std::make_unique<Private>()) {
        d_ptr->startIndex = startIndex;
        d_ptr->originalSize = originalSize;
    }

    /*!
        Writes plugin information to stream, returns \c true if success.
    */
    bool PluginFileWriter::save(const std::filesystem::path &path) const {
        std::ofstream fs(path);
        if (!fs.is_open())
            return false;

        std::ostream &os = fs;

        auto d = d_ptr.get();

        struct NoteItem {
            bool removed = false;
            const Note *changed = nullptr;
            const std::vector<Note> *inserted = nullptr;
        };

        // Collect data
        std::vector<NoteItem> noteItems(d->originalSize + 1); // Reserved the last for insertion
        for (const auto &item : d->changedNotes) {
            noteItems[item.first - d->startIndex].changed = &item.second;
        }
        for (const auto &item : d->insertedNotes) {
            noteItems[item.first - d->startIndex].inserted = &item.second;
        }
        for (const auto &item : d->removedNotes) {
            noteItems[item - d->startIndex].removed = true;
        }

        // Previous
        if (!d->notesBeforePrev.empty()) {
            for (const auto &note : d->notesBeforePrev) {
                writeSectionName(SECTION_NAME_INSERT, os);
                writeSectionNote(-1, note, os);
            }

            // Complement
            if (!d->prevNote) {
                writeSectionName(SECTION_NAME_PREV, os);
            }
        }
        if (d->prevNote) {
            writeSectionName(SECTION_NAME_PREV, os);
            writeSectionNote(-1, d->prevNote.value(), os);
        }

        // Selection
        for (int i = 0; i < noteItems.size(); ++i) {
            const auto &item = noteItems.at(i);
            if (item.inserted) {
                for (const auto &note : *item.inserted) {
                    writeSectionName(SECTION_NAME_INSERT, os);
                    writeSectionNote(-1, note, os);
                }
            }

            // Skip last
            if (i + 1 == noteItems.size())
                break;

            if (item.removed) {
                writeSectionName(SECTION_NAME_DELETE, os);
                continue;
            }

            int idx = d->startIndex + i;
            if (item.changed) {
                writeSectionNote(idx, *item.changed, os);
                continue;
            }

            // Keep index
            writeSectionName(idx, os);
        }

        // Next
        if (d->nextNote) {
            writeSectionName(SECTION_NAME_NEXT, os);
            writeSectionNote(-1, d->nextNote.value(), os);
        }
        if (!d->notesAfterNext.empty()) {
            // Complement
            if (!d->nextNote) {
                writeSectionName(SECTION_NAME_NEXT, os);
            }

            for (const auto &note : d->notesAfterNext) {
                writeSectionName(SECTION_NAME_INSERT, os);
                writeSectionNote(-1, note, os);
            }
        }
        return true;
    }

    void PluginFileWriter::setNote(int index, const Note &note) {
        d_ptr->changedNotes[index] = note;
    }

    void PluginFileWriter::setPrevNote(const Note &note) {
        d_ptr->prevNote = note;
    }

    void PluginFileWriter::setNextNote(const Note &note) {
        d_ptr->nextNote = note;
    }

    void PluginFileWriter::insertNotes(int index, const std::vector<Note> &notes) {
        auto &vec = d_ptr->insertedNotes[index];
        vec.insert(vec.end(), notes.begin(), notes.end());
    }

    void PluginFileWriter::prependNotesBeforePrev(const std::vector<Note> &notes) {
        auto &vec = d_ptr->notesBeforePrev;
        vec.insert(vec.begin(), notes.begin(), notes.end());
    }

    void PluginFileWriter::appendNotesAfterNext(const std::vector<Note> &notes) {
        auto &vec = d_ptr->notesAfterNext;
        vec.insert(vec.end(), notes.begin(), notes.end());
    }

    void PluginFileWriter::removeNote(int index) {
        d_ptr->removedNotes.insert(index);
    }

    void PluginFileWriter::removeNotes(const std::vector<int> &indexes) {
        for (const auto &idx : indexes)
            d_ptr->removedNotes.insert(idx);
    }

}