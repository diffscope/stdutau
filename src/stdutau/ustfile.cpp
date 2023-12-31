#include "ustfile.h"

#include <fstream>

#include "utautils.h"

namespace Utau {

    static inline Note createInitialNote() {
        Note note;

        // These properties have explicit default values when created by editor
        // We need to reset them when reading the file
        note.intensity = NODEF_DOUBLE;
        note.modulation = NODEF_DOUBLE;

        return note;
    }

    static inline void getDouble(const std::string_view &s, double &out) {
        out = stod2(s, out);
    }

    static inline double getDouble(const std::string_view &s) {
        return stod2(s);
    }

    static inline void getInt(const std::string_view &s, int &out) {
        out = stoi2(s, out);
    }

    static inline int getInt(const std::string_view &s) {
        return stoi2(s);
    }


    static bool parseSectionName(const std::string_view &str, std::string_view &name) {
        if (starts_with(str, SECTION_BEGIN_MARK) && ends_with(str, SECTION_END_MARK)) {
            name = str.substr(sizeof(SECTION_BEGIN_MARK) - 1, str.size() -
                                                                  (sizeof(SECTION_BEGIN_MARK) - 1) -
                                                                  (sizeof(SECTION_END_MARK) - 1));
            return true;
        }
        return false;
    }

    static void parseSectionNote(const std::vector<std::string> &sectionList, Note &note) {
        PBStrings mode2;

        for (const std::string_view &line : sectionList) {
            auto eq = line.find('=');
            if (eq == std::string_view::npos) {
                continue;
            }

            auto key = line.substr(0, eq);
            auto value = line.substr(eq + 1);
            if (key == KEY_NAME_LYRIC) {
                note.lyric = value;                // Lyric
            } else if (key == KEY_NAME_NOTE_NUM) {
                getInt(value, note.noteNum);       // Note Num
            } else if (key == KEY_NAME_LENGTH) {
                getInt(value, note.length);        // Length
            } else if (key == KEY_NAME_FLAGS) {
                note.flags = value;                // Flags
            } else if (key == KEY_NAME_INTENSITY) {
                getDouble(value, note.intensity);  // Intensity
            } else if (key == KEY_NAME_MODULATION || key == KEY_NAME_MODURATION) {
                getDouble(value, note.modulation); // Modulation
            } else if (key == KEY_NAME_PRE_UTTERANCE) {
                getDouble(value, note.preUttr);    // PreUtterence
            } else if (key == KEY_NAME_VOICE_OVERLAP) {
                getDouble(value, note.overlap);    // Overlap
            } else if (key == KEY_NAME_VELOCITY) {
                getDouble(value, note.velocity);   // Consonant Velocity
            } else if (key == KEY_NAME_START_POINT) {
                getDouble(value, note.stp);        // StartPoint
            } else if (key == KEY_NAME_TEMPO) {
                getDouble(value, note.tempo);      // Tempo
            } else if (key == KEY_NAME_REGION_START) {
                note.region = value;               // Start of region
            } else if (key == KEY_NAME_REGION_END) {
                note.regionEnd = value;            // End of region
            } else if (key == KEY_NAME_PB_START) {
                getDouble(value, note.pbstart);    // Mode1 Start
            } else if (key == KEY_NAME_PBS) {
                mode2.PBS = value;                 // Mode2 Start
            } else if (key == KEY_NAME_PBW) {
                mode2.PBW = value;                 // Mode2 Intervals
            } else if (key == KEY_NAME_PBY) {
                mode2.PBY = value;                 // Mode2 Offsets
            } else if (key == KEY_NAME_PBM) {
                mode2.PBM = value;                 // Mode2 Types
            } else if (key == KEY_NAME_PICHES || key == KEY_NAME_PITCHES ||
                       key == KEY_NAME_PITCH_BEND) {
                note.pitches = stringsToDoubles(split(value, ","));       // Mode1 Pitch
            } else if (key == KEY_NAME_VBR) {
                note.vibrato = Vibrato::fromString(std::string(value));   // Vibrato
            } else if (key == KEY_NAME_ENVELOPE) {
                note.envelope = Envelope::fromString(std::string(value)); // Envelope
            }
        }
        note.portamento = mode2.toPoints(); // Mode2 Pitch
    }

    static void parseSectionVersion(const std::vector<std::string> &sectionList, UstVersion &out) {
        for (const std::string_view &line : sectionList) {
            auto eq = line.find('=');
            if (eq != std::string_view::npos) {
                auto key = line.substr(0, eq);
                auto value = line.substr(eq + 1);
                if (key == KEY_NAME_CHARSET) {
                    out.charset = value;
                }
                continue;
            }

            if (line.find(UST_VERSION_PREFIX_NOSPACE) == 0) {
                out.version = line.substr(sizeof(UST_VERSION_PREFIX_NOSPACE) - 1);
            }
        }
    }

    static void parseSectionSettings(const std::vector<std::string> &sectionList,
                                     UstSettings &out) {
        for (const std::string_view &section : sectionList) {
            auto eq = section.find("=");
            if (eq == std::string::npos) {
                continue;
            }

            auto key = section.substr(0, eq);
            auto value = section.substr(eq + 1);
            if (key == KEY_NAME_PROJECT_NAME) {
                out.projectName = value;      // Project Name
            } else if (key == KEY_NAME_OUTPUT_FILE) {
                out.outputFileName = value;   // Output File Name
            } else if (key == KEY_NAME_VOICE_DIR) {
                out.voiceDir = value;         // Voice Directory
            } else if (key == KEY_NAME_CACHE_DIR) {
                out.cacheDir = value;         // Cache Directory
            } else if (key == KEY_NAME_TOOL1) {
                out.wavtoolPath = value;      // Wavtool
            } else if (key == KEY_NAME_TOOL2) {
                out.resamplerPath = value;    // Resampler
            } else if (key == KEY_NAME_MODE2) {
                out.isMode2 = true;           // Mode2
            } else if (key == KEY_NAME_TEMPO) {
                out.tempo = getDouble(value); // Global Tempo
                if (out.tempo < VALUE_TEMPO_MIN || out.tempo > VALUE_TEMPO_MAX) {
                    out.tempo = DEFAULT_VALUE_TEMPO;
                }
            } else if (key == KEY_NAME_FLAGS) {
                out.flags = value; // Flags
            }
        }
    }

    static void writeSectionName(const std::string &name, std::ostream &out) {
        out << SECTION_BEGIN_MARK + name + SECTION_END_MARK << std::endl;
    }

    static void writeSectionName(int name, std::ostream &out) {
        auto newName = to_string(name);
        int nums = newName.size();
        if (nums < 4) {
            newName = std::string(4 - nums, '0') + newName;
        }
        writeSectionName(newName, out);
    }

    static void writeSectionNote(int num, const Note &note, std::ostream &out) {
        writeSectionName(num, out);

        // Items always exists
        out << KEY_NAME_LENGTH << "=" << note.length << std::endl;
        out << KEY_NAME_LYRIC << "=" << note.lyric << std::endl;
        out << KEY_NAME_NOTE_NUM << "=" << note.noteNum << std::endl;

        // Items can be omitted
        if (note.preUttr != NODEF_DOUBLE) {
            out << KEY_NAME_PRE_UTTERANCE << "=" << note.preUttr << std::endl;
        } else {
            // UST files always keep this property even if empty
            out << KEY_NAME_PRE_UTTERANCE << "=" << std::endl;
        }
        if (note.overlap != NODEF_DOUBLE) {
            out << KEY_NAME_VOICE_OVERLAP << "=" << note.overlap << std::endl;
        }
        if (note.velocity != NODEF_DOUBLE) {
            out << KEY_NAME_VELOCITY << "=" << to_string(note.velocity) << std::endl;
        }
        if (note.intensity != NODEF_DOUBLE) {
            out << KEY_NAME_INTENSITY << "=" << note.intensity << std::endl;
        }
        if (note.modulation != NODEF_DOUBLE) {
            out << KEY_NAME_MODULATION << "=" << note.modulation << std::endl;
        }
        if (note.stp != NODEF_DOUBLE) {
            out << KEY_NAME_START_POINT << "=" << note.stp << std::endl;
        }
        if (!note.flags.empty()) {
            out << KEY_NAME_FLAGS << "=" << note.flags << std::endl;
        }

        // Items may not exist
        if (!note.pitches.empty()) {
            out << KEY_NAME_PB_TYPE << "=5" << std::endl;
            out << KEY_NAME_PB_START << "=" << note.pbstart << std::endl;
            out << KEY_NAME_PITCH_BEND << "=" << join(doublesToStrings(note.pitches), ",")
                << std::endl;
        }

        if (note.envelope) {
            out << KEY_NAME_ENVELOPE << "=" << note.envelope->toString() << std::endl;
        }

        if (!note.portamento.empty()) {
            auto mode2 = PBStrings::fromPoints(note.portamento);
            out << KEY_NAME_PBS << "=" << mode2.PBS << std::endl;
            out << KEY_NAME_PBW << "=" << mode2.PBW << std::endl;
            if (!mode2.PBY.empty()) {
                out << KEY_NAME_PBY << "=" << mode2.PBY << std::endl;
            }
            if (!mode2.PBM.empty()) {
                out << KEY_NAME_PBM << "=" << mode2.PBM << std::endl;
            }
        }
        if (note.vibrato) {
            out << KEY_NAME_VBR << "=" << note.vibrato->toString() << std::endl;
        }
        if (note.tempo != NODEF_DOUBLE) {
            out << KEY_NAME_TEMPO << "=" << note.tempo << std::endl;
        }
        if (!note.region.empty()) {
            out << KEY_NAME_REGION_START << "=" << note.region << std::endl;
        }
        if (!note.regionEnd.empty()) {
            out << KEY_NAME_REGION_END << "=" << note.regionEnd << std::endl;
        }
    }

    static void writeSectionVersion(const UstVersion &version, std::ostream &out) {
        writeSectionName(SECTION_NAME_VERSION, out);

        out << UST_VERSION_PREFIX_NOSPACE << version.version << std::endl;

        // UTF-8 UST File?
        std::string charset = version.charset;
        if (!charset.empty()) {
            out << KEY_NAME_CHARSET << "=" << charset << std::endl;
        }
    }

    static void writeSectionSettings(const UstSettings &settings, std::ostream &out) {
        writeSectionName(SECTION_NAME_SETTING, out);

        out << KEY_NAME_TEMPO << "=" << settings.tempo << std::endl;
        out << KEY_NAME_TRACKS << "=1" << std::endl;
        out << KEY_NAME_PROJECT_NAME << "=" << settings.projectName << std::endl;
        out << KEY_NAME_VOICE_DIR << "=" << settings.voiceDir << std::endl;
        out << KEY_NAME_OUTPUT_FILE << "=" << settings.outputFileName << std::endl;
        out << KEY_NAME_CACHE_DIR << "=" << settings.cacheDir << std::endl;
        out << KEY_NAME_TOOL1 << "=" << settings.wavtoolPath << std::endl;
        out << KEY_NAME_TOOL2 << "=" << settings.resamplerPath << std::endl;

        if (settings.isMode2) {
            out << KEY_NAME_MODE2 << "=" << VALUE_MODE2_ON << std::endl;
        }

        if (!settings.flags.empty()) {
            out << KEY_NAME_FLAGS << "=" << settings.flags << std::endl;
        }
    }

    UstFile::UstFile() = default;

    bool UstFile::read(std::istream &is) {
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