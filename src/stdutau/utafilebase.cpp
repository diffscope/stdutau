#include "utafilebase.h"

#include <fstream>

namespace Utau {

    UtaFileBase::UtaFileBase() = default;

    UtaFileBase::~UtaFileBase() = default;

    /*!
        Reads the specific file, returns \c true if succees.
    */
    bool UtaFileBase::load(const std::filesystem::path &path) {
        std::ifstream fs(path);
        if (!fs.is_open())
            return false;
        return read(fs);
    }

    /*!
        Writes the specifc file, returns \c true if succees.
    */
    bool UtaFileBase::save(const std::filesystem::path &path) const {
        std::ofstream fs(path);
        if (!fs.is_open())
            return false;
        return write(fs);
    }

}