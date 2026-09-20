#include "prefixmap.h"

#include <fstream>

#include "utautils.h"
#include "utaconst.h"

namespace utau {

    PrefixMap::PrefixMap() = default;

    bool PrefixMap::load(const std::filesystem::path &path) {
        std::ifstream fs(path);
        if (!fs.is_open())
            return false;
        return read(fs);
    }

    bool PrefixMap::save(const std::filesystem::path &path) const {
        std::ofstream fs(path);
        if (!fs.is_open())
            return false;
        return write(fs);
    }

    bool PrefixMap::read(std::istream &is) {
        static const constexpr int min = TONE_NUMBER_BASE;
        static const constexpr int max =
            min + (TONE_OCTAVE_MAX - TONE_OCTAVE_MIN + 1) * TONE_OCTAVE_STEPS - 1;

        std::string line;
        while (readLine(is, line)) {
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

    bool PrefixMap::write(std::ostream &os) const {
        for (auto it = map.begin(); it != map.end(); ++it) {
            int key = it->first;
            os << toneNumToToneName(key) << "\t" << it->second.prefix << "\t" << it->second.suffix
               << std::endl;
            if (!os.good())
                return false;
        }
        return true;
    }

    std::string PrefixMap::prefixedLyric(int noteNum, const std::string &lyric) const {
        auto it = map.find(noteNum);
        if (it == map.end()) {
            return lyric;
        }
        return it->second.prefix + lyric + it->second.suffix;
    }

}