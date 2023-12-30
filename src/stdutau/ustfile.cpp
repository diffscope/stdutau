#include "ustfile.h"

#include <fstream>

namespace Utau {

    UstFile::UstFile() = default;

    bool UstFile::read(std::istream &is) {
        return true;
    }

    bool UstFile::write(std::ostream &os) const {
        return true;
    }
}