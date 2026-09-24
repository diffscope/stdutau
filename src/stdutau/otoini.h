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
    /// \sa http://utau2008.blog47.fc2.com/blog-entry-425.html
    ///     UTAU 0.4.10, which reads and writes an \c oto.ini in UTF-8
    /// \sa GetOtoDeclaredEncoding() of OpenUtau, which reads the declaration in \a charset . No
    ///     release contains it yet, so the link refers to a commit:
    ///     https://github.com/stakira/OpenUtau/blob/56eafb7060443cddcd9d7da240be2dd92c216713/OpenUtau.Core/Classic/VoicebankLoader.cs
    class STDUTAU_EXPORT OtoIni {
    public:
        OtoIni();

        /// Opens and reads \a path . Returns \c false if the file cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Creates and writes \a path . Returns \c false if the file cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// Reads one entry per line. Missing trailing fields are filled with zeros, and a line
        /// without a sample file name is skipped. The declaration of the encoding is read into
        /// \a charset and is not an entry.
        bool read(std::string_view text);

        /// Writes the declaration of the encoding, if any, followed by the entries grouped by
        /// sample file, the files in ascending order.
        ///
        /// The original order is deliberately not preserved: UTAU also sorts the entries when it
        /// saves an \c oto.ini , so no voice bank can rely on its order surviving a save.
        ///
        /// A number retains its original text. See OtoEntry::spellings .
        std::string write() const;

    public:
        /// The encoding that the file declares for itself on a line of the form
        /// \c #Charset:UTF-8 , or empty if it declares none.
        ///
        /// The name is kept as written and is not interpreted. The line is recognized regardless
        /// of case, and written as the first line in the form \c #Charset: followed by the name.
        /// Dropping it would make a program that honors it read a UTF-8 file in the code page of
        /// the machine. The sources are listed with the class.
        std::string charset;

        /// Entries keyed by sample file name. A file has one entry per alias, which is the common
        /// case rather than an exception.
        std::map<std::string, std::vector<OtoEntry>> contents;
    };

}

#endif // OTOINI_H
