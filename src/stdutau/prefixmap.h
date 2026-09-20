#ifndef PREFIXMAP_H
#define PREFIXMAP_H

#include <map>
#include <string>
#include <vector>

#include <stdutau/utafilebase.h>

namespace Utau {

    /// A voice bank's \c prefix.map, which decides what is added to a lyric at a given key.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    class STDUTAU_EXPORT PrefixMap : public UtaFileBase {
    public:
        PrefixMap();

        /// Reads tab separated lines of tone name, prefix and suffix. A line naming a key outside
        /// C1 to B7 is skipped, as is one with fewer than three fields.
        bool read(std::istream &is) override;

        /// Writes one tab separated line per key, in ascending order.
        bool write(std::ostream &os) const override;

    public:
        /// What goes before and after the lyric at one key.
        struct Item {
            std::string prefix;
            std::string suffix;
        };

        /// Keyed by note number, where 24 is C1.
        std::map<int, Item> map;

        /// Returns \a lyric wrapped in the prefix and suffix registered for \a noteNum, or \a
        /// lyric unchanged when that key has no entry.
        std::string prefixedLyric(int noteNum, const std::string &lyric) const;
    };

}

#endif // PREFIXMAP_H
