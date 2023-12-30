#ifndef OTOINI_H
#define OTOINI_H

#include <map>
#include <vector>
#include <filesystem>

#include <stdutau/genonsettings.h>

namespace Utau {

    class STDUTAU_EXPORT OtoIni {
    public:
        OtoIni();

        bool load(const std::filesystem::path &path);
        bool save(const std::filesystem::path &path) const;

        std::map<std::string, std::vector<GenonSettings>> contents;
    };

}

#endif // OTOINI_H
