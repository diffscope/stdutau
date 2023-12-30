#ifndef UTAFILEBASE_H
#define UTAFILEBASE_H

#include <map>
#include <vector>
#include <filesystem>
#include <iostream>

#include <stdutau/utaglobal.h>

namespace Utau {

    class STDUTAU_EXPORT UtaFileBase {
    public:
        UtaFileBase();
        virtual ~UtaFileBase();

        bool load(const std::filesystem::path &path);
        bool save(const std::filesystem::path &path) const;

        virtual bool read(std::istream &is) = 0;
        virtual bool write(std::ostream &os) const = 0;
    };

}

#endif // UTAFILEBASE_H
