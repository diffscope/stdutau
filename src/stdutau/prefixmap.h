#ifndef PREFIXMAP_H
#define PREFIXMAP_H

#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include <stdutau/utaglobal.h>

namespace Utau {

    class STDUTAU_EXPORT PrefixMap {
    public:
        PrefixMap();

        bool load(const std::filesystem::path &path);
        bool save(const std::filesystem::path &path) const;

        struct Item {
            std::string prefix;
            std::string suffix;
        };
        std::map<int, Item> map;
    };

}

#endif // PREFIXMAP_H
