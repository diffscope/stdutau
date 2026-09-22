#ifndef UTAUTILS_H
#define UTAUTILS_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// The line terminator written by this library, which is the one UTAU writes.
    ///
    /// Independent of the platform, because a UTAU file must be identical regardless of the
    /// system that wrote it.
    constexpr const char LINE_END[] = "\r\n";

    /// Removes the first line from \a text into \a line , without the terminator, and returns
    /// whether a line was available.
    ///
    /// \code
    ///   std::string_view line;
    ///   while (takeLine(text, line)) {
    ///       // text now begins after that line
    ///   }
    /// \endcode
    ///
    /// \note Both terminators are accepted. This library reads bytes rather than a text-mode
    ///       stream, so a CRLF file is read unchanged on every platform, and every reader in
    ///       this library uses this function instead of handling terminators itself.
    STDUTAU_EXPORT bool takeLine(std::string_view &text, std::string_view &line);

    /// Splits at every occurrence of \a delimiter , keeping empty fields. The views refer into
    /// \a s , which must outlive them.
    STDUTAU_EXPORT std::vector<std::string_view> split(const std::string_view &s,
                                                       const std::string_view &delimiter);

    /// Joins the elements of \a v with \a delimiter .
    STDUTAU_EXPORT std::string join(const std::vector<std::string_view> &v,
                                    const std::string_view &delimiter);
    STDUTAU_EXPORT std::string join(const std::vector<std::string> &v,
                                    const std::string_view &delimiter);

    /// Removes the whitespace at both ends of \a s .
    STDUTAU_EXPORT std::string trim(const std::string &s);

    inline bool starts_with(const std::string_view &s, const std::string_view &prefix) {
#if __cplusplus >= 202002L
        return s.starts_with(prefix);
#else
        return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
#endif
    }

    inline bool ends_with(const std::string_view &s, const std::string_view &prefix) {
#if __cplusplus >= 202002L
        return s.ends_with(prefix);
#else
        return s.size() >= prefix.size() && s.substr(s.size() - prefix.size()) == prefix;
#endif
    }

    /// Parses a number from \a s , or returns \a defaultValue if \a s is not a number. The two
    /// cases are indistinguishable, so use toInt() and toDouble() if the distinction matters.
    STDUTAU_EXPORT int stoi2(const std::string_view &s, int defaultValue = 0);
    STDUTAU_EXPORT double stod2(const std::string_view &s, double defaultValue = 0);

    /// Parses a number from \a s , or returns \c std::nullopt if \a s is not a number.
    ///
    /// Preferred over stoi2() and stod2() if an absent entry must be distinguished from an entry
    /// with the value zero.
    STDUTAU_EXPORT std::optional<int> toInt(const std::string_view &s);
    STDUTAU_EXPORT std::optional<double> toDouble(const std::string_view &s);

    /// Formats \a num through a stream at its default precision, so a double is written with at
    /// most six significant digits.
    STDUTAU_EXPORT std::string to_string(double num);
    STDUTAU_EXPORT std::string to_string(int num);

    /// Converts a list of numbers, reading any non-numeric element as zero.
    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string> &strs);
    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string_view> &strs);

    /// Writes a zero as an empty string and removes trailing empty elements, which is how a UST
    /// shortens a pitch curve.
    STDUTAU_EXPORT std::vector<std::string> doublesToStrings(const std::vector<double> &nums);

    /// Converts between a key and its name, where 24 is C1. A name shorter than two characters
    /// is read as C1, an unknown letter as C, and an octave outside 1 to 7 is clamped to that
    /// range.
    STDUTAU_EXPORT int toneNameToToneNum(const std::string_view &name);
    STDUTAU_EXPORT std::string toneNumToToneName(int num);

    /// Builds a name from the position within the octave and the octave, both zero-based.
    /// Returns an empty string if \a nameIndex is outside 0 to 11.
    STDUTAU_EXPORT std::string toneNumToToneName(int nameIndex, int octaveIndex);

    /// Converts between ticks and milliseconds at \a tempo, where 480 ticks is a quarter note.
    STDUTAU_EXPORT double tickToTime(int tick, double tempo);
    STDUTAU_EXPORT int timeToTick(double time, double tempo);

    /// Returns whether \a oLyric denotes a rest: \c R , \c r or the empty string.
    STDUTAU_EXPORT bool isRestLyric(const std::string &oLyric);

}

#endif // UTAUTILS_H
