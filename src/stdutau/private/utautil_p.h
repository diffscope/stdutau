#ifndef UTAUTIL_P_H
#define UTAUTIL_P_H

#include <string>

namespace utau {

    /// Returns the shortest fixed-notation text of \a value that toDouble() reads back as
    /// \a value , with a period as the decimal separator.
    std::string formatFixed(double value);

}

#endif // UTAUTIL_P_H
