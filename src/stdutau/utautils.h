#ifndef UTAUTILS_H
#define UTAUTILS_H

#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace Utau {

    STDUTAU_EXPORT std::vector<std::string_view> split(const std::string_view &s,
                                                       const std::string_view &delimiter);
    STDUTAU_EXPORT std::string join(const std::vector<std::string_view> &v,
                                    const std::string_view &delimiter);
    STDUTAU_EXPORT std::string join(const std::vector<std::string> &v,
                                    const std::string_view &delimiter);
    STDUTAU_EXPORT std::string trim(const std::string &s);

    STDUTAU_EXPORT int stoi2(const std::string_view &s, int defaultValue = 0);
    STDUTAU_EXPORT double stod2(const std::string_view &s, double defaultValue = 0);

    STDUTAU_EXPORT std::vector<double> stringsToDoubles(const std::vector<std::string> &strs);
    STDUTAU_EXPORT std::vector<std::string> doublesToStrings(const std::vector<double> &nums);

    STDUTAU_EXPORT int toneNameToToneNum(const std::string_view &name);
    STDUTAU_EXPORT std::string toneNumToToneName(int num);
    STDUTAU_EXPORT std::string toneNumToToneName(int nameIndex, int octaveIndex);

    STDUTAU_EXPORT double tickToTime(int tick, double tempo);
    STDUTAU_EXPORT int timeToTick(double time, double tempo);
    STDUTAU_EXPORT bool isRestLyric(const std::string &oLyric);

}

#endif // UTAUTILS_H
