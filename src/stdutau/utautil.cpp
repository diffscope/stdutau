#include "utautils.h"

#include <charconv>

#include "utaconst.h"

namespace Utau {

    std::vector<std::string_view> split(const std::string_view &s,
                                        const std::string_view &delimiter) {
        std::vector<std::string_view> tokens;
        std::string_view::size_type start = 0;
        std::string_view::size_type end = s.find(delimiter);
        while (end != std::string_view::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.size();
            end = s.find(delimiter, start);
        }
        tokens.push_back(s.substr(start));
        return tokens;
    }

    std::string join(const std::vector<std::string_view> &v, const std::string_view &delimiter) {
        if (v.empty())
            return {};

        std::string res;
        for (int i = 0; i < v.size() - 1; ++i) {
            res.append(v[i]);
            res.append(delimiter);
        }
        res.append(v.back());
        return res;
    }

    std::string join(const std::vector<std::string> &v, const std::string_view &delimiter) {
        if (v.empty())
            return {};

        std::string res;
        for (int i = 0; i < v.size() - 1; ++i) {
            res.append(v[i]);
            res.append(delimiter);
        }
        res.append(v.back());
        return res;
    }

    std::string trim(const std::string &s) {
        auto start = s.begin();
        while (start != s.end() && std::isspace(*start)) {
            start++;
        }

        auto end = s.end();
        do {
            end--;
        } while (std::distance(start, end) > 0 && std::isspace(*end));

        return {start, end + 1};
    }

    int stoi2(const std::string_view &s, int defaultValue) {
        std::from_chars(s.data(), s.data() + s.size(), defaultValue);
        return defaultValue;
    }

    double stod2(const std::string_view &s, double defaultValue) {
        std::from_chars(s.data(), s.data() + s.size(), defaultValue);
        return defaultValue;
    }

    std::vector<double> stringsToDoubles(const std::vector<std::string> &strs) {
        std::vector<double> nums;
        for (const auto &s : strs) {
            nums.push_back(stod2(s));
        }
        return nums;
    }

    std::vector<double> stringsToDoubles(const std::vector<std::string_view> &strs) {
        std::vector<double> nums;
        for (const auto &s : strs) {
            nums.push_back(stod2(s));
        }
        return nums;
    }

    std::vector<std::string> doublesToStrings(const std::vector<double> &nums) {
        std::vector<std::string> strs;
        for (const auto &num : nums) {
            strs.push_back(num == 0 ? std::string() : std::to_string(num));
        }
        while (!strs.empty() && strs.back().empty()) {
            strs.pop_back();
        }
        return strs;
    }

    int toneNameToToneNum(const std::string_view &name) {
        if (name.length() < 2) {
            return Utau::TONE_NUMBER_BASE;
        }

        int index = std::string_view(TONE_NAMES).find(name.front());
        if (index == std::string_view::npos) {
            index = 0;
        }

        int octave = name.back() - '0';
        if (octave < TONE_OCTAVE_MIN) {
            octave = TONE_OCTAVE_MIN;
        } else if (octave > TONE_OCTAVE_MAX) {
            octave = TONE_OCTAVE_MAX;
        }

        return TONE_NUMBER_BASE + (octave - 1) * TONE_OCTAVE_STEPS + index +
               static_cast<int>(name[1] == TONE_NAME_SHARP);
    }

    std::string toneNumToToneName(int num) {
        return toneNumToToneName(num % TONE_OCTAVE_STEPS, num / TONE_OCTAVE_STEPS - 2);
    }

    std::string toneNumToToneName(int nameIndex, int octaveIndex) {
        std::string tone_names(TONE_NAMES);
        std::string name;
        name += tone_names.at(nameIndex);
        if (nameIndex > 0 && tone_names.at(nameIndex) == tone_names.at(nameIndex - 1)) {
            name += TONE_NAME_SHARP;
        }
        name += std::to_string(octaveIndex + 1);
        return name;
    }

    double tickToTime(int tick, double tempo) {
        return (static_cast<unsigned long>(60000) * tick) / (tempo * TIME_BASE);
    }

    int timeToTick(double time, double tempo) {
        return static_cast<int>(time * tempo * TIME_BASE / 60000);
    }

    bool isRestLyric(const std::string &lyric) {
        std::string lrc = trim(lyric);
        return lrc.empty() || lrc == "R" || lrc == "r";
    }

}