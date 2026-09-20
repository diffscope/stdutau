#include "ustfile.h"

#include <fstream>
#include <algorithm>

#include "utautils.h"
#include "private/usthelper_p.h"

namespace Utau {

    inline Note createInitialNote() {
        Note note;

        // The constructor gives these the values an editor starts a note with. A note being read
        // carries only what the file says.
        note.intensity.reset();
        note.modulation.reset();

        return note;
    }

    UstFile::UstFile() = default;

    bool UstFile::load(const std::filesystem::path &path) {
        std::ifstream fs(path);
        if (!fs.is_open())
            return false;
        return read(fs);
    }

    bool UstFile::save(const std::filesystem::path &path) const {
        std::ofstream fs(path);
        if (!fs.is_open())
            return false;
        return write(fs);
    }

    bool UstFile::read(std::istream &is) {
        // Read File
        std::vector<std::string> currentSection;

        std::string line;
        while (readLine(is, line)) {
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
                    parseSectionVersion(currentSection, version);
                } else if (sectionName == SECTION_NAME_SETTING) {
                    // Parse global settings
                    parseSectionSettings(currentSection, settings);
                } else if (std::all_of(sectionName.begin(), sectionName.end(), ::isdigit)) {
                    // Parse Note (Name should be numeric)
                    auto note = createInitialNote();
                    parseSectionNote(currentSection, note);
                    // Ignore note whose length is invalid
                    if (note.length > 0) {
                        notes.push_back(note);
                    }
                }
            }

            currentSection.clear();
            currentSection.push_back(line);
        }
        return true;
    }

    bool UstFile::write(std::ostream &os) const {
        writeSectionVersion(version, os);   // Write Version
        writeSectionSettings(settings, os); // Write Global Settings

        if (!os.good())
            return false;

        // Write Notes
        for (int i = 0; i < notes.size(); ++i) {
            writeSectionNote(i, notes.at(i), os);
            if (!os.good())
                return false;
        }

        writeSectionName(SECTION_NAME_TRACKEND, os); // Write End Sign
        if (!os.good())
            return false;
        return true;
    }

}