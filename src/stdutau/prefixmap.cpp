#include "prefixmap.h"

#include <fstream>

#include "utautils.h"
#include "utaconst.h"

namespace Utau {

    /*!
        Constructor.
    */
    PrefixMap::PrefixMap() = default;

    /*!
        Reads \c prefix.map items from file, returns \c true if succees.
    */
    bool PrefixMap::load(const std::filesystem::path &path) {
        std::ifstream fs(path);
        if (!fs.is_open())
            return false;

        static const constexpr int min = TONE_NUMBER_BASE;
        static const constexpr int max =
            min + (TONE_OCTAVE_MAX - TONE_OCTAVE_MIN + 1) * TONE_OCTAVE_STEPS - 1;

        std::string line;
        while (std::getline(fs, line)) {
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

    /*!
        Writes \c prefix.map map items to file, returns \c true if succees.
    */
    bool PrefixMap::save(const std::filesystem::path &path) const {
        std::ofstream fs(path);
        if (!fs.is_open())
            return false;

        for (auto it = map.begin(); it != map.end(); ++it) {
            int key = it->first;
            fs << toneNumToToneName(key) << "\t" << it->second.prefix << "\t" << it->second.suffix
               << std::endl;
        }
        return true;
    }

}