#ifndef OTOINI_H
#define OTOINI_H

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/otoentry.h>

namespace utau {

    /// One \c oto.ini of a voice bank. A voice bank usually has several, one per directory.
    ///
    /// The strings are raw bytes. They must be converted from the file encoding before use.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/106.html
    ///     原音設定, which defines the entries below
    class STDUTAU_EXPORT OtoIni {
    public:
        OtoIni();

        /// Opens and reads \a path . Returns \c false if the file cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Creates and writes \a path . Returns \c false if the file cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// Reads one entry per line. Missing trailing fields are filled with zeros, and a line
        /// without a sample file name is skipped.
        bool read(std::string_view text);

        /// Writes the entries grouped by sample file, the files in ascending order.
        ///
        /// The original order is deliberately not preserved: UTAU also sorts the entries when it
        /// saves an \c oto.ini , so no voice bank can rely on its order surviving a save.
        ///
        /// A number retains its original text. See OtoEntry::spellings .
        std::string write() const;

    public:
        /// Entries keyed by sample file name. A file has one entry per alias, which is the common
        /// case rather than an exception.
        std::map<std::string, std::vector<OtoEntry>> contents;
    };

}

#endif // OTOINI_H
