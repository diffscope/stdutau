#ifndef UTAUTILS_H
#define UTAUTILS_H

#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// Reads one line from \a is into \a line with the line terminator removed, whichever of the
    /// two it is, and returns whether a line was read.
    ///
    /// \note UTAU writes CRLF. A stream opened in text mode turns that into a newline on Windows
    ///       and leaves it alone everywhere else, so \c std::getline hands back a line ending in
    ///       a carriage return on every other platform. Every reader in this library goes through
    ///       here so that it does not have to care which platform it is on.
    STDUTAU_EXPORT bool readLine(std::istream &is, std::string &line);

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
    STDUTAU_EXPORT std::string toneNumToToneName(int nameIndex, int octaveIndex);

    /// Converts between ticks and milliseconds at \a tempo, where 480 ticks is a quarter note.
    STDUTAU_EXPORT double tickToTime(int tick, double tempo);
    STDUTAU_EXPORT int timeToTick(double time, double tempo);

    /// Whether \a oLyric is a rest, which \c R , \c r and the empty string all are.
    STDUTAU_EXPORT bool isRestLyric(const std::string &oLyric);

}

#endif // UTAUTILS_H
