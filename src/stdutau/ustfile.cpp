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
        // Whether the file ends without a terminator, which determines when the loop below
        // reaches the last line. In a file ending with a terminator, nothing follows the final
        // section marker, so the marker closes the preceding section rather than opening one.
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
            currentSection.emplace_back(line);
        }
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