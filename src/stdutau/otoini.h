#ifndef OTOINI_H
#define OTOINI_H

#include <filesystem>
#include <iosfwd>
#include <map>
#include <vector>

#include <stdutau/otoentry.h>

namespace utau {

    /// One \c oto.ini of a voice bank. A bank usually has several, one per directory.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    class STDUTAU_EXPORT OtoIni {
    public:
        OtoIni();

        /// Opens \a path and reads it, returns \c false when the file will not open.
        bool load(const std::filesystem::path &path);

        /// Creates \a path and writes to it, returns \c false when the file will not open.
        bool save(const std::filesystem::path &path) const;

        /// Reads one entry per line. A line missing its trailing fields is filled out with zeros,
        /// and one naming no sample file is skipped.
        bool read(std::istream &is);

        /// Writes the entries grouped by sample file, the files in ascending order.
        bool write(std::ostream &os) const;

    public:
        /// Entries keyed by sample file name. One file carries as many entries as it has aliases,
        /// which is the ordinary shape of a bank rather than an oddity.
        std::map<std::string, std::vector<OtoEntry>> contents;
    };

}

#endif // OTOINI_H
