#include "utafilebase.h"

#include <fstream>

namespace Utau {

    /*!
        \class UtaFileBase
        \brief Basic class of filesystem IO for this library.
    */

    /*!
        Constructor.
    */
    UtaFileBase::UtaFileBase() = default;

    /*!
        Destructor.
    */
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

    /*!
        \fn bool UtaFileBase::read(std::istream &is)

        Reads the spefific file contents from the stream.
    */

    /*!
        \fn bool UtaFileBase::write(std::ostream &os) const

        Writes the spefific file contents to the stream.
    */

}