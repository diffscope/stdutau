#include "usthelper_p.h"

#include "utautils.h"

namespace utau {

    static inline void getDouble(const std::string_view &s, double &out) {
        out = stod2(s, out);
    }

    // Leaves \a out as it was when the text holds no number, so an entry that is present but
    // empty stays absent rather than turning into zero.
    static inline void getDouble(const std::string_view &s, std::optional<double> &out) {
        if (auto value = toDouble(s)) {
            out = value;
        }
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

    // Entries a note carries that parseSectionNote() reads under another name, or that
    // writeSectionNote() derives on its own. They are not unknown, so they must stay out of
    // Note::userData, which would otherwise write them a second time.
    static inline bool isReservedKey(const std::string_view &key) {
        return key == KEY_NAME_PB_TYPE || key == KEY_NAME_PRE_UTTERANCE_READONLY ||
               key == KEY_NAME_VOICE_OVERLAP_READONLY || key == KEY_NAME_START_POINT_READONLY ||
               key == KEY_NAME_FILENAME_READONLY || key == KEY_NAME_ALIAS_READONLY ||
               key == KEY_NAME_CACHE_READONLY;
    }

    bool parseSectionName(const std::string_view &str, std::string_view &name) {
        if (starts_with(str, SECTION_BEGIN_MARK) && ends_with(str, SECTION_END_MARK)) {
            name = str.substr(sizeof(SECTION_BEGIN_MARK) - 1, str.size() -
                                                                  (sizeof(SECTION_BEGIN_MARK) - 1) -
                                                                  (sizeof(SECTION_END_MARK) - 1));
            return true;
        }
        return false;
    }

    void parseSectionNote(const std::vector<std::string> &sectionList, Note &note) {
        PBStrings mode2;

        for (const auto &item : sectionList) {
            std::string_view line = item;
            auto eq = line.find('=');
            if (eq == std::string_view::npos) {
                continue;
            }

            auto key = line.substr(0, eq);
            auto value = line.substr(eq + 1);
            if (key == KEY_NAME_LYRIC) {
                note.lyric = value; // Lyric
            } else if (key == KEY_NAME_NOTE_NUM) {
                getInt(value, note.noteNum); // Note Num
            } else if (key == KEY_NAME_LENGTH) {
                getInt(value, note.length); // Length
            } else if (key == KEY_NAME_FLAGS) {
                note.flags = value; // Flags
            } else if (key == KEY_NAME_INTENSITY) {
                getDouble(value, note.intensity); // Intensity
            } else if (key == KEY_NAME_MODULATION || key == KEY_NAME_MODURATION) {
                getDouble(value, note.modulation); // Modulation
            } else if (key == KEY_NAME_PRE_UTTERANCE) {
                getDouble(value, note.preUttr); // PreUtterance
            } else if (key == KEY_NAME_VOICE_OVERLAP) {
                getDouble(value, note.overlap); // Overlap
            } else if (key == KEY_NAME_VELOCITY) {
                getDouble(value, note.velocity); // Consonant Velocity
            } else if (key == KEY_NAME_START_POINT) {
                getDouble(value, note.stp); // StartPoint
            } else if (key == KEY_NAME_TEMPO) {
                getDouble(value, note.tempo); // Tempo
            } else if (key == KEY_NAME_REGION_START) {
                note.region = value; // Start of region
            } else if (key == KEY_NAME_REGION_END) {
                note.regionEnd = value; // End of region
            } else if (key == KEY_NAME_PB_START) {
                getDouble(value, note.pbstart); // Mode1 Start
            } else if (key == KEY_NAME_PBS) {
                mode2.PBS = value; // Mode2 Start
            } else if (key == KEY_NAME_PBW) {
                mode2.PBW = value; // Mode2 Intervals
            } else if (key == KEY_NAME_PBY) {
                mode2.PBY = value; // Mode2 Offsets
            } else if (key == KEY_NAME_PBM) {
                mode2.PBM = value; // Mode2 Types
            } else if (key == KEY_NAME_PICHES || key == KEY_NAME_PITCHES ||
                       key == KEY_NAME_PITCH_BEND) {
                note.pitches = stringsToDoubles(split(value, ",")); // Mode1 Pitch
            } else if (key == KEY_NAME_VBR) {
                note.vibrato = Vibrato::fromString(std::string(value)); // Vibrato
            } else if (key == KEY_NAME_ENVELOPE) {
                note.envelope = Envelope::fromString(std::string(value)); // Envelope
            } else if (key == KEY_NAME_LABEL) {
                note.label = value; // Label
            } else if (key == KEY_NAME_DIRECT) {
                note.direct = value; // Direct rendering
            } else if (key == KEY_NAME_PATCH) {
                note.patch = value; // Patch
            } else if (!isReservedKey(key)) {
                note.userData[std::string(key)] = value; // Anything else
            }
        }
        note.portamento = mode2.toPoints(); // Mode2 Pitch
    }

    void parseSectionNoteExt(const std::vector<std::string> &sectionList, NoteExt &note) {
        parseSectionNote(sectionList, note);

        for (const auto &item : sectionList) {
            std::string_view line = item;
            auto eq = line.find('=');
            if (eq == std::string_view::npos) {
                continue;
            }

            auto key = line.substr(0, eq);
            auto value = line.substr(eq + 1);

            if (key == KEY_NAME_PRE_UTTERANCE_READONLY) {
                getDouble(value, note.preUttrRO); // PreUtterance
            } else if (key == KEY_NAME_VOICE_OVERLAP_READONLY) {
                getDouble(value, note.overlapRO); // Overlap
            } else if (key == KEY_NAME_START_POINT_READONLY) {
                getDouble(value, note.stpRO); // StartPoint
            } else if (key == KEY_NAME_FILENAME_READONLY) {
                note.filenameRO = value; // @filename
            } else if (key == KEY_NAME_ALIAS_READONLY) {
                note.aliasRO = value; // @readonly
            } else if (key == KEY_NAME_CACHE_READONLY) {
                note.cacheRO = value; // @cache
            }
        }
    }

    void parseSectionVersion(const std::vector<std::string> &sectionList, UstVersion &out) {
        for (const auto &item : sectionList) {
            std::string_view line = item;
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

    void parseSectionSettings(const std::vector<std::string> &sectionList, UstSettings &out) {
        for (const auto &item : sectionList) {
            std::string_view line = item;
            auto eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }

            auto key = line.substr(0, eq);
            auto value = line.substr(eq + 1);
            if (key == KEY_NAME_PROJECT_NAME) {
                out.projectName = value; // Project Name
            } else if (key == KEY_NAME_OUTPUT_FILE) {
                out.outputFileName = value; // Output File Name
            } else if (key == KEY_NAME_VOICE_DIR) {
                out.voiceDir = value; // Voice Directory
            } else if (key == KEY_NAME_CACHE_DIR) {
                out.cacheDir = value; // Cache Directory
            } else if (key == KEY_NAME_TOOL1) {
                out.wavtoolPath = value; // Wavtool
            } else if (key == KEY_NAME_TOOL2) {
                out.resamplerPath = value; // Resampler
            } else if (key == KEY_NAME_MODE2) {
                out.isMode2 = true; // Mode2
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

    // Appends one "key=value" line, which is nearly every line a UST is made of.
    static void writeEntry(std::string &out, std::string_view key, std::string_view value) {
        out += key;
        out += '=';
        out += value;
        out += LINE_END;
    }

    void writeSectionName(const std::string &name, std::string &out) {
        out += SECTION_BEGIN_MARK + name + SECTION_END_MARK;
        out += LINE_END;
    }

    void writeSectionName(int name, std::string &out) {
        auto newName = to_string(name);
        auto nums = newName.size();
        if (nums < 4) {
            newName = std::string(4 - nums, '0') + newName;
        }
        writeSectionName(newName, out);
    }

    void writeSectionNote(int num, const Note &note, std::string &out) {
        if (num >= 0) {
            writeSectionName(num, out);
        }

        // Items always exists
        writeEntry(out, KEY_NAME_LENGTH, to_string(note.length));
        writeEntry(out, KEY_NAME_LYRIC, note.lyric);
        writeEntry(out, KEY_NAME_NOTE_NUM, to_string(note.noteNum));

        // Items can be omitted
        if (note.preUttr) {
            writeEntry(out, KEY_NAME_PRE_UTTERANCE, to_string(*note.preUttr));
        } else {
            // UST files always keep this property even if empty
            writeEntry(out, KEY_NAME_PRE_UTTERANCE, {});
        }
        if (note.overlap) {
            writeEntry(out, KEY_NAME_VOICE_OVERLAP, to_string(*note.overlap));
        }
        if (note.velocity) {
            writeEntry(out, KEY_NAME_VELOCITY, to_string(*note.velocity));
        }
        if (note.intensity) {
            writeEntry(out, KEY_NAME_INTENSITY, to_string(*note.intensity));
        }
        if (note.modulation) {
            writeEntry(out, KEY_NAME_MODULATION, to_string(*note.modulation));
        }
        if (note.stp) {
            writeEntry(out, KEY_NAME_START_POINT, to_string(*note.stp));
        }
        if (!note.flags.empty()) {
            writeEntry(out, KEY_NAME_FLAGS, note.flags);
        }

        // Items may not exist
        if (!note.pitches.empty()) {
            writeEntry(out, KEY_NAME_PB_TYPE, VALUE_PITCH_TYPE);
            writeEntry(out, KEY_NAME_PB_START, to_string(note.pbstart.value_or(0)));
            writeEntry(out, KEY_NAME_PITCH_BEND, join(doublesToStrings(note.pitches), ","));
        }

        if (note.envelope) {
            writeEntry(out, KEY_NAME_ENVELOPE, note.envelope->toString());
        }

        if (!note.portamento.empty()) {
            auto mode2 = PBStrings::fromPoints(note.portamento);
            writeEntry(out, KEY_NAME_PBS, mode2.PBS);
            writeEntry(out, KEY_NAME_PBW, mode2.PBW);
            if (!mode2.PBY.empty()) {
                writeEntry(out, KEY_NAME_PBY, mode2.PBY);
            }
            if (!mode2.PBM.empty()) {
                writeEntry(out, KEY_NAME_PBM, mode2.PBM);
            }
        }
        if (note.vibrato) {
            writeEntry(out, KEY_NAME_VBR, note.vibrato->toString());
        }
        if (note.tempo) {
            writeEntry(out, KEY_NAME_TEMPO, to_string(*note.tempo));
        }
        if (!note.region.empty()) {
            writeEntry(out, KEY_NAME_REGION_START, note.region);
        }
        if (!note.regionEnd.empty()) {
            writeEntry(out, KEY_NAME_REGION_END, note.regionEnd);
        }
        if (!note.label.empty()) {
            writeEntry(out, KEY_NAME_LABEL, note.label);
        }
        if (!note.direct.empty()) {
            writeEntry(out, KEY_NAME_DIRECT, note.direct);
        }
        if (!note.patch.empty()) {
            writeEntry(out, KEY_NAME_PATCH, note.patch);
        }

        // Last, which is where UTAU puts the entries it did not recognize when it writes a file
        // back. Matching that keeps a file this library wrote and the same file after a pass
        // through UTAU in the same shape.
        for (const auto &pair : note.userData) {
            writeEntry(out, pair.first, pair.second);
        }
    }

    void writeSectionVersion(const UstVersion &version, std::string &out) {
        writeSectionName(SECTION_NAME_VERSION, out);

        out += UST_VERSION_PREFIX_NOSPACE;
        out += version.version;
        out += LINE_END;

        // UTF-8 UST File?
        if (!version.charset.empty()) {
            writeEntry(out, KEY_NAME_CHARSET, version.charset);
        }
    }

    void writeSectionSettings(const UstSettings &settings, std::string &out) {
        writeSectionName(SECTION_NAME_SETTING, out);

        writeEntry(out, KEY_NAME_TEMPO, to_string(settings.tempo));
        writeEntry(out, KEY_NAME_TRACKS, VALUE_PROJECT_TRACKS);
        writeEntry(out, KEY_NAME_PROJECT_NAME, settings.projectName);
        writeEntry(out, KEY_NAME_VOICE_DIR, settings.voiceDir);
        writeEntry(out, KEY_NAME_OUTPUT_FILE, settings.outputFileName);
        writeEntry(out, KEY_NAME_CACHE_DIR, settings.cacheDir);
        writeEntry(out, KEY_NAME_TOOL1, settings.wavtoolPath);
        writeEntry(out, KEY_NAME_TOOL2, settings.resamplerPath);

        if (settings.isMode2) {
            writeEntry(out, KEY_NAME_MODE2, VALUE_MODE2_ON);
        }

        if (!settings.flags.empty()) {
            writeEntry(out, KEY_NAME_FLAGS, settings.flags);
        }
    }

}
