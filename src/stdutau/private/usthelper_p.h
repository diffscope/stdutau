#ifndef USTHELPER_P_H
#define USTHELPER_P_H

#include <vector>
#include <string>
#include <iostream>

#include <stdutau/note.h>
#include <stdutau/ustfile.h>

namespace Utau {

    bool parseSectionName(const std::string_view &str, std::string_view &name);
    void parseSectionNote(const std::vector<std::string> &sectionList, Note &note);
    void parseSectionNoteExt(const std::vector<std::string> &sectionList, NoteExt &note);
    void parseSectionVersion(const std::vector<std::string> &sectionList, UstVersion &out);
    void parseSectionSettings(const std::vector<std::string> &sectionList, UstSettings &out);

    void writeSectionName(const std::string &name, std::ostream &out);
    void writeSectionName(int name, std::ostream &out);
    void writeSectionNote(int num, const Note &note, std::ostream &out);
    void writeSectionVersion(const UstVersion &version, std::ostream &out);
    void writeSectionSettings(const UstSettings &settings, std::ostream &out);

}

#endif // USTHELPER_P_H
