#include "utautils.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <charconv>

#ifdef _LIBCPP_VERSION
#  include <clocale>
#  ifdef __APPLE__
#    include <xlocale.h>
#  endif
#endif

#include "private/utautil_p.h"
#include "utaconst.h"

namespace utau {

#ifdef _LIBCPP_VERSION
    // The C locale, in which strtod reads a period as the decimal separator. strtod itself reads
    // the separator of the current locale, which a host program selects with setlocale. A Qt
    // program, for example, calls setlocale(LC_ALL, "") on Unix, and a German locale then makes
    // strtod read "12.5" as 12. The locale lasts for the lifetime of the program.
#  ifdef _WIN32
    static _locale_t cLocale() {
        static const _locale_t locale = _create_locale(LC_NUMERIC, "C");
        return locale;
    }
#  else
    static locale_t cLocale() {
        static const locale_t locale = newlocale(LC_NUMERIC_MASK, "C", locale_t(0));
        return locale;
    }
#  endif
#endif

    // Parses a double without exceptions, because this library is built without them.
    //
    // libc++ does not yet provide a floating-point std::from_chars, so that path falls back to
    // strtod in the C locale. strtod requires a terminated string, which a string_view does not
    // guarantee, hence the copy.
    static bool parseDouble(const std::string_view &s, double &out) {
#ifdef _LIBCPP_VERSION
        // std::from_chars accepts neither leading whitespace nor a leading plus sign, whereas
        // strtod accepts both. They are rejected here so that both paths behave identically.
        if (s.empty() || s.front() == '+' || isAsciiSpace(s.front())) {
            return false;
        }

        std::string str(s);
        const char *begin = str.c_str();
        char *end = nullptr;
        errno = 0;
#  ifdef _WIN32
        double value = _strtod_l(begin, &end, cLocale());
#  else
        double value = strtod_l(begin, &end, cLocale());
#  endif
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

    // libc++ provides the floating-point std::to_chars only from macOS 13.3. Its path writes the
    // number with snprintf in the C locale at increasing precisions and keeps the first text that
    // parseDouble() reads back as the value, the text with the fewest digits after the point.
    // Above 2^53, where a double has no digits after the point, the text holds the exact integral
    // value rather than the shortest digits of std::to_chars.
    std::string formatFixed(double value) {
        char buffer[400];
#ifdef _LIBCPP_VERSION
        // The smallest subnormal double requires 324 digits after the point.
        constexpr int maxPrecision = 340;
        int length = 0;
        for (int precision = 0; precision <= maxPrecision; ++precision) {
#  ifdef _WIN32
            length = _snprintf_l(buffer, sizeof(buffer), "%.*f", cLocale(), precision, value);
#  else
            const locale_t previous = uselocale(cLocale());
            length = std::snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
            uselocale(previous);
#  endif
            double read;
            if (length > 0 && parseDouble(std::string_view(buffer, length), read) &&
                read == value) {
                break;
            }
        }
        return std::string(buffer, length > 0 ? size_t(length) : 0);
#else
        const auto result =
            std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::fixed);
        return std::string(buffer, result.ptr);
#endif
    }

    bool takeLine(std::string_view &text, std::string_view &line) {
        if (text.empty()) {
            return false;
        }

        const auto end = text.find_first_of("\r\n");
        if (end == std::string_view::npos) {
            line = text;
            text = {};
            return true;
        }
        line = text.substr(0, end);
        const auto next = text.find_first_not_of("\r\n", end);
        text = next == std::string_view::npos ? std::string_view() : text.substr(next);
        return true;
    }

    bool takeLineKeepingEmpty(std::string_view &text, std::string_view &line) {
        if (text.empty()) {
            return false;
        }

        const auto end = text.find_first_of("\r\n");
        if (end == std::string_view::npos) {
            line = text;
            text = {};
            return true;
        }
        line = text.substr(0, end);
        auto rest = text.substr(end);
        if (rest.substr(0, 3) == "\r\r\n") {
            rest.remove_prefix(3);
        } else if (rest.substr(0, 2) == "\r\n") {
            rest.remove_prefix(2);
        } else {
            rest.remove_prefix(1);
        }
        text = rest;
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
        auto start = s.begin();
        while (start != s.end() && isAsciiSpace(*start)) {
            start++;
        }

        auto end = s.end();
        while (end != start && isAsciiSpace(*(end - 1))) {
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
        // U+3000 IDEOGRAPHIC SPACE in UTF-8
        constexpr std::string_view fullWidthSpace = "\xE3\x80\x80";
        std::string_view lrc = lyric;
        for (bool trimmed = true; trimmed;) {
            trimmed = false;
            while (!lrc.empty() && isAsciiSpace(lrc.front())) {
                lrc.remove_prefix(1);
                trimmed = true;
            }
            while (!lrc.empty() && isAsciiSpace(lrc.back())) {
                lrc.remove_suffix(1);
                trimmed = true;
            }
            if (starts_with(lrc, fullWidthSpace)) {
                lrc.remove_prefix(fullWidthSpace.size());
                trimmed = true;
            }
            if (ends_with(lrc, fullWidthSpace)) {
                lrc.remove_suffix(fullWidthSpace.size());
                trimmed = true;
            }
        }
        return lrc.empty() || lrc == "R" || lrc == "r";
    }

}
