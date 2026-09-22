#include "utautils.h"

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <sstream>
#include <string>
#include <charconv>

#include "utaconst.h"

namespace utau {

    // Parses a double without exceptions, because this library is built without them.
    //
    // libc++ does not yet provide a floating-point std::from_chars, so that path falls back to
    // strtod. strtod requires a terminated string, which a string_view does not guarantee, hence
    // the copy.
    static bool parseDouble(const std::string_view &s, double &out) {
#ifdef _LIBCPP_VERSION
        // std::from_chars accepts neither leading whitespace nor a leading plus sign, whereas
        // strtod accepts both. They are rejected here so that both paths behave identically.
        if (s.empty() || s.front() == '+' || std::isspace(static_cast<unsigned char>(s.front()))) {
            return false;
        }

        std::string str(s);
        const char *begin = str.c_str();
        char *end = nullptr;
        errno = 0;
        double value = std::strtod(begin, &end);
        if (end == begin || errno == ERANGE) {
            return false;
        }
        out = value;
        return true;
#else
        auto result = std::from_chars(s.data(), s.data() + s.size(), out);
        return result.ec == std::errc();
#endif
    }

    bool takeLine(std::string_view &text, std::string_view &line) {
        if (text.empty()) {
            return false;
        }

        const auto end = text.find('\n');
        if (end == std::string_view::npos) {
            line = text;
            text = {};
        } else {
            line = text.substr(0, end);
            text = text.substr(end + 1);
        }
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        return true;
    }

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
        // The cast is required. This library handles raw bytes, and std::isspace on a negative
        // value other than EOF is undefined, which applies to every byte above 0x7F of a
        // Shift_JIS string.
        auto isSpace = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };

        auto start = s.begin();
        while (start != s.end() && isSpace(*start)) {
            start++;
        }

        auto end = s.end();
        while (end != start && isSpace(*(end - 1))) {
            end--;
        }

        return {start, end};
    }

    int stoi2(const std::string_view &s, int defaultValue) {
        std::from_chars(s.data(), s.data() + s.size(), defaultValue);
        return defaultValue;
    }

    double stod2(const std::string_view &s, double defaultValue) {
        double value;
        if (!parseDouble(s, value)) {
            return defaultValue;
        }
        return value;
    }

    std::optional<int> toInt(const std::string_view &s) {
        int value;
        auto result = std::from_chars(s.data(), s.data() + s.size(), value);
        if (result.ec != std::errc()) {
            return std::nullopt;
        }
        return value;
    }

    std::optional<double> toDouble(const std::string_view &s) {
        double value;
        if (!parseDouble(s, value)) {
            return std::nullopt;
        }
        return value;
    }

    std::string to_string(double num) {
        std::ostringstream ss;
        ss << num;
        return ss.str();
    }

    std::string to_string(int num) {
        std::ostringstream ss;
        ss << num;
        return ss.str();
    }

    std::vector<double> stringsToDoubles(const std::vector<std::string> &strs) {
        std::vector<double> nums;
        nums.reserve(strs.size());
        for (const auto &s : strs) {
            nums.push_back(stod2(s));
        }
        return nums;
    }

    std::vector<double> stringsToDoubles(const std::vector<std::string_view> &strs) {
        std::vector<double> nums;
        nums.reserve(strs.size());
        for (const auto &s : strs) {
            nums.push_back(stod2(s));
        }
        return nums;
    }

    std::vector<std::string> doublesToStrings(const std::vector<double> &nums) {
        std::vector<std::string> strs;
        strs.reserve(nums.size());
        for (const auto &num : nums) {
            strs.push_back(num == 0 ? std::string() : to_string(num));
        }
        while (!strs.empty() && strs.back().empty()) {
            strs.pop_back();
        }
        return strs;
    }

    int toneNameToToneNum(const std::string_view &name) {
        if (name.length() < 2) {
            return TONE_NUMBER_BASE;
        }

        auto index = std::string_view(TONE_NAMES).find(name.front());
        if (index == std::string_view::npos) {
            index = 0;
        }

        int octave = name.back() - '0';
        if (octave < TONE_OCTAVE_MIN) {
            octave = TONE_OCTAVE_MIN;
        } else if (octave > TONE_OCTAVE_MAX) {
            octave = TONE_OCTAVE_MAX;
        }

        return TONE_NUMBER_BASE + (octave - 1) * TONE_OCTAVE_STEPS + int(index) +
               static_cast<int>(name[1] == TONE_NAME_SHARP);
    }

    std::string toneNumToToneName(int num) {
        return toneNumToToneName(num % TONE_OCTAVE_STEPS, num / TONE_OCTAVE_STEPS - 2);
    }

    std::string toneNumToToneName(int nameIndex, int octaveIndex) {
        std::string_view tone_names(TONE_NAMES);
        if (nameIndex < 0 || nameIndex >= int(tone_names.size())) {
            return {};
        }

        std::string name;
        name += tone_names[nameIndex];
        if (nameIndex > 0 && tone_names[nameIndex] == tone_names[nameIndex - 1]) {
            name += TONE_NAME_SHARP;
        }
        name += to_string(octaveIndex + 1);
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
