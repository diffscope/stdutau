#ifndef PREFIXMAP_H
#define PREFIXMAP_H

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// A voice bank's \c prefix.map, which decides what is added to a lyric at a given key.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/107.html
    ///     多音階音源の作り方, on what this file is for
    class STDUTAU_EXPORT PrefixMap {
    public:
        PrefixMap();

        /// Opens \a path and reads it, returns \c false when the file will not open.
        bool load(const std::filesystem::path &path);

        /// Creates \a path and writes to it, returns \c false when the file will not open.
        bool save(const std::filesystem::path &path) const;

        /// Reads tab separated lines of tone name, prefix and suffix. A line naming a key outside
        /// C1 to B7 is skipped, as is one with fewer than three fields.
        bool read(std::string_view text);

        /// Writes one tab separated line per key, in ascending order.
        std::string write() const;

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
