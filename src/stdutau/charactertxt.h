#ifndef CHARACTERTXT_H
#define CHARACTERTXT_H

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// The \c character.txt of a voice bank, which contains the information UTAU displays about it.
    ///
    /// The strings are raw bytes. They must be converted from the file encoding before use.
    ///
    /// \note The official UTAU site does not document this file. The two pages below are the
    ///       references used by Japanese voice bank authors, and the entries follow them.
    ///
    /// \sa https://w.atwiki.jp/vbmaker/pages/70.html
    ///     character.txt, UTAU音源制作wiki
    /// \sa https://tatsu3.hateblo.jp/entry/ar343137
    ///     原音のプロパティについて, which the page above references for the entries
    class STDUTAU_EXPORT CharacterTxt {
    public:
        CharacterTxt();

        /// Opens and reads \a path . Returns \c false if the file cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Creates and writes \a path . Returns \c false if the file cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// Reads one \c key=value pair per line. Any other line is stored in \a extraLines .
        bool read(std::string_view text);

        /// Writes the present entries, then \a extraLines . An empty entry is omitted, so that an
        /// unspecified entry is absent rather than empty.
        std::string write() const;

    public:
        /// The display name, which replaces the folder name used for a voice bank without one.
        std::string name;

        /// The icon file, relative to the directory. UTAU expects a 100 by 100 bitmap.
        std::string image;

        /// The audio file played by the Sample button. Without one, UTAU plays an arbitrary sample.
        std::string sample;

        std::string author;
        std::string web;

        /// Lines without a dedicated member, preserved as read and written after the entries
        /// above.
        ///
        /// A \c character.txt contains more than entries: UTAU displays a line containing a colon
        /// as part of the character profile, which is why voice banks contain lines such as
        /// \c Version:1.0 . These lines are content, and dropping them would delete author data.
        std::vector<std::string> extraLines;
    };

}

#endif // CHARACTERTXT_H
