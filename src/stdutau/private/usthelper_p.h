#ifndef USTHELPER_P_H
#define USTHELPER_P_H

#include <vector>
#include <string>

#include <stdutau/note.h>
#include <stdutau/ustfile.h>

namespace utau {

    bool parseSectionName(const std::string_view &str, std::string_view &name);
    void parseSectionNote(const std::vector<std::string> &sectionList, Note &note);
    void parseSectionNoteExt(const std::vector<std::string> &sectionList, NoteExt &note);
    void parseSectionVersion(const std::vector<std::string> &sectionList, UstVersion &out);
    void parseSectionSettings(const std::vector<std::string> &sectionList, UstSettings &out);

    /// Whether \a key is a note entry that parseSectionNote() does not store under its own name:
    /// the read-only \c @ entries and \c PBType , which writeSectionNote() derives.
    bool isReservedKey(const std::string_view &key);

    /// Appends one \c key=value line.
    void writeEntry(std::string &out, std::string_view key, std::string_view value);

    void writeSectionName(const std::string &name, std::string &out);
    void writeSectionName(int name, std::string &out);
    void writeSectionNote(int num, const Note &note, std::string &out);

    /// Writes the note, then its read-only \c @ entries.
    void writeSectionNoteExt(int num, const NoteExt &note, std::string &out);
    void writeSectionVersion(const UstVersion &version, std::string &out);
    void writeSectionSettings(const UstSettings &settings, std::string &out);

}

#endif // USTHELPER_P_H
