#include "prefixmap.h"

#include <fstream>

#include "utautils.h"
#include "utaconst.h"

namespace utau {

    PrefixMap::PrefixMap() = default;

    bool PrefixMap::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool PrefixMap::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    bool PrefixMap::read(std::string_view text) {
        static const constexpr int min = TONE_NUMBER_BASE;
        static const constexpr int max =
            min + (TONE_OCTAVE_MAX - TONE_OCTAVE_MIN + 1) * TONE_OCTAVE_STEPS - 1;

        std::string_view line;
        while (takeLine(text, line)) {
            if (line.empty()) {
                continue;
            }

            auto tokens = split(line, "\t");
            if (tokens.size() < 3) {
                continue;
            }
            int noteNum = toneNameToToneNum(tokens[0]);
            if (noteNum >= min && noteNum <= max) {
                map[noteNum] = Item{
                    std::string(tokens[1]),
                    std::string(tokens[2]),
                };
            }
        }
        return true;
    }

    std::string PrefixMap::write() const {
        std::string out;
        for (auto it = map.begin(); it != map.end(); ++it) {
            out += toneNumToToneName(it->first);
            out += '\t';
            out += it->second.prefix;
            out += '\t';
            out += it->second.suffix;
            out += LINE_END;
        }
        return out;
    }

    std::string PrefixMap::prefixedLyric(int noteNum, const std::string &lyric) const {
        auto it = map.find(noteNum);
        if (it == map.end()) {
            return lyric;
        }
        return it->second.prefix + lyric + it->second.suffix;
    }

}