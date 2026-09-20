#ifndef OTOINI_H
#define OTOINI_H

#include <map>
#include <vector>

#include <stdutau/utafilebase.h>
#include <stdutau/otoentry.h>

namespace Utau {

    /// One \c oto.ini of a voice bank. A bank usually has several, one per directory.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    class STDUTAU_EXPORT OtoIni : public UtaFileBase {
    public:
        OtoIni();

        /// Reads one entry per line. A line missing its trailing fields is filled out with zeros,
        /// and one naming no sample file is skipped.
        bool read(std::istream &is) override;

        /// Writes the entries grouped by sample file, the files in ascending order.
        bool write(std::ostream &os) const override;

    public:
        /// Entries keyed by sample file name. One file carries as many entries as it has aliases,
        /// which is the ordinary shape of a bank rather than an oddity.
        std::map<std::string, std::vector<OtoEntry>> contents;
    };

}

#endif // OTOINI_H
