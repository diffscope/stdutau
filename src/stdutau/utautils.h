#ifndef UTAUTILS_H
#define UTAUTILS_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// The line terminator this library writes, which is the one UTAU writes.
    ///
    /// Not the platform's. A UTAU file is the same file on every system, and which system wrote
    /// it is not supposed to show.
    constexpr const char LINE_END[] = "\r\n";

    /// Takes the next line off the front of \a text into \a line, with the terminator removed,
    /// and returns whether there was one.
    ///
    /// \code
    ///   std::string_view line;
    ///   while (takeLine(text, line)) {
    ///       // text now begins after that line
    ///   }
    /// \endcode
    ///
    /// \note Either terminator is accepted. This library reads bytes rather than a text mode
    ///       stream, so a CRLF file arrives as it was written on every platform, and every
    ///       reader here goes through this rather than working that out again.
    STDUTAU_EXPORT bool takeLine(std::string_view &text, std::string_view &line);

    /// Splits on every occurrence of \a delimiter, keeping empty fields. The views point into
    /// \a s, which has to outlive them.
    STDUTAU_EXPORT std::vector<std::string_view> split(const std::string_view &s,
                                                       const std::string_view &delimiter);

    /// Puts \a delimiter between the elements of \a v.
    STDUTAU_EXPORT std::string join(const std::vector<std::string_view> &v,
                                    const std::string_view &delimiter);
    STDUTAU_EXPORT std::string join(const std::vector<std::string> &v,
                                    const std::string_view &delimiter);

    /// Drops the whitespace at both ends of \a s.
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

    /// Reads a number from \a s, or returns \a defaultValue when \a s does not hold one. The two
    /// answers are the same value, so reach for toInt() and toDouble() where that matters.
    STDUTAU_EXPORT int stoi2(const std::string_view &s, int defaultValue = 0);
    STDUTAU_EXPORT double stod2(const std::string_view &s, double defaultValue = 0);

    /// Reads a number from \a s, or nothing when \a s does not hold one.
    ///
    /// Prefer these over stoi2() and stod2() wherever an entry being absent has to stay apart
    /// from the entry holding zero.
    STDUTAU_EXPORT std::optional<int> toInt(const std::string_view &s);
    STDUTAU_EXPORT std::optional<double> toDouble(const std::string_view &s);

    /// Writes \a num through a stream at its default precision, so a double keeps six significant
    /// digits and no more.
    STDUTAU_EXPORT std::string to_string(double num);
    STDUTAU_EXPORT std::string to_string(int num);

    /// Converts a list of numbers, reading anything that is not one as zero.
    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string> &strs);
    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string_view> &strs);

    /// Writes a zero as an empty string and drops the empty ones off the end, which is how a UST
    /// keeps a pitch curve short.
    STDUTAU_EXPORT std::vector<std::string> doublesToStrings(const std::vector<double> &nums);

    /// Converts between a key and its name, where 24 is C1. A name shorter than two characters
    /// reads as C1, an unknown letter reads as C, and an octave outside 1 to 7 is pulled back
    /// into that range.
    STDUTAU_EXPORT int toneNameToToneNum(const std::string_view &name);
    STDUTAU_EXPORT std::string toneNumToToneName(int num);

    /// Builds a name from a position within the octave and the octave, both counted from zero.
    /// Returns an empty string when \a nameIndex is outside 0 to 11.
    STDUTAU_EXPORT std::string toneNumToToneName(int nameIndex, int octaveIndex);

    /// Converts between ticks and milliseconds at \a tempo, where 480 ticks is a quarter note.
    STDUTAU_EXPORT double tickToTime(int tick, double tempo);
    STDUTAU_EXPORT int timeToTick(double time, double tempo);

    /// Whether \a oLyric is a rest, which \c R , \c r and the empty string all are.
    STDUTAU_EXPORT bool isRestLyric(const std::string &oLyric);

}

#endif // UTAUTILS_H
