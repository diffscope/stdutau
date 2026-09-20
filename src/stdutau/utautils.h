#ifndef UTAUTILS_H
#define UTAUTILS_H

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace Utau {

    /// Reads one line from \a is into \a line with the line terminator removed, whichever of the
    /// two it is, and returns whether a line was read.
    ///
    /// \note UTAU writes CRLF. A stream opened in text mode turns that into a newline on Windows
    ///       and leaves it alone everywhere else, so \c std::getline hands back a line ending in
    ///       a carriage return on every other platform. Every reader in this library goes through
    ///       here so that it does not have to care which platform it is on.
    STDUTAU_EXPORT bool readLine(std::istream &is, std::string &line);

    STDUTAU_EXPORT std::vector<std::string_view> split(const std::string_view &s,
                                                       const std::string_view &delimiter);
    STDUTAU_EXPORT std::string join(const std::vector<std::string_view> &v,
                                    const std::string_view &delimiter);
    STDUTAU_EXPORT std::string join(const std::vector<std::string> &v,
                                    const std::string_view &delimiter);
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

    STDUTAU_EXPORT int stoi2(const std::string_view &s, int defaultValue = 0);
    STDUTAU_EXPORT double stod2(const std::string_view &s, double defaultValue = 0);

    STDUTAU_EXPORT std::string to_string(double num);
    STDUTAU_EXPORT std::string to_string(int num);

    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string> &strs);
    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string_view> &strs);
    STDUTAU_EXPORT std::vector<std::string> doublesToStrings(const std::vector<double> &nums);

    STDUTAU_EXPORT int toneNameToToneNum(const std::string_view &name);
    STDUTAU_EXPORT std::string toneNumToToneName(int num);
    STDUTAU_EXPORT std::string toneNumToToneName(int nameIndex, int octaveIndex);

    STDUTAU_EXPORT double tickToTime(int tick, double tempo);
    STDUTAU_EXPORT int timeToTick(double time, double tempo);
    STDUTAU_EXPORT bool isRestLyric(const std::string &oLyric);

}

#endif // UTAUTILS_H
