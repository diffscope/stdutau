#ifndef PREFIXMAP_H
#define PREFIXMAP_H

#include <map>
#include <string>
#include <vector>

#include <stdutau/utafilebase.h>

namespace Utau {

    class STDUTAU_EXPORT PrefixMap : public UtaFileBase {
    public:
        PrefixMap();

        bool read(std::istream &is) override;
        bool write(std::ostream &os) const override;

    public:
        struct Item {
            std::string prefix;
            std::string suffix;
        };
        std::map<int, Item> map;

        std::string prefixedLyric(int noteNum, const std::string &lyric) const;
    };

}

#endif // PREFIXMAP_H
