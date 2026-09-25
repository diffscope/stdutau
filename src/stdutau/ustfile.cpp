#include "ustfile.h"

#include <fstream>
#include <algorithm>

#include "utautils.h"
#include "private/usthelper_p.h"

namespace utau {

    inline Note createInitialNote() {
        Note note;

        // The constructor initializes these with the values of a new note in an editor. A note
        // read from a file contains only the values the file specifies.
        note.intensity.reset();
        note.modulation.reset();

        return note;
    }

    UstFile::UstFile() = default;

    bool UstFile::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool UstFile::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    bool UstFile::read(std::string_view text) {
        // The lines of the current section, its header first. Lines before the first header
        // belong to no section.
        std::vector<std::string> currentSection;

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
            } else if (std::all_of(sectionName.begin(), sectionName.end(), isAsciiDigit)) {
                // A note, whose section is named by its number.
                auto note = createInitialNote();
                parseSectionNote(currentSection, note);
                // A note without a valid length is ignored.
                if (note.length > 0) {
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

    std::string UstFile::write() const {
        std::string out;
        writeSectionVersion(version, out);   // Write Version
        writeSectionSettings(settings, out); // Write Global Settings

        // Write Notes
        for (int i = 0; i < notes.size(); ++i) {
            writeSectionNote(i, notes[i], out);
        }

        writeSectionName(SECTION_NAME_TRACKEND, out); // Write End Sign
        return out;
    }

}