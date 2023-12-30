#ifndef OTOINI_H
#define OTOINI_H

#include <map>
#include <vector>

#include <stdutau/utafilebase.h>
#include <stdutau/genonsettings.h>

namespace Utau {

    class STDUTAU_EXPORT OtoIni : public UtaFileBase {
    public:
        OtoIni();

        bool read(std::istream &is) override;
        bool write(std::ostream &os) const override;

    public:
        std::map<std::string, std::vector<GenonSettings>> contents;
    };

}

#endif // OTOINI_H
