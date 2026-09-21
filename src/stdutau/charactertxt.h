#ifndef CHARACTERTXT_H
#define CHARACTERTXT_H

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// A voice bank's \c character.txt , which is what UTAU shows about the bank.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    ///
    /// \note UTAU's own site does not write this one down. The two below are what the Japanese
    ///       voice bank authors go by, and the entries here are theirs.
    ///
    /// \sa https://w.atwiki.jp/vbmaker/pages/70.html
    ///     character.txt, UTAU音源制作wiki
    /// \sa https://tatsu3.hateblo.jp/entry/ar343137
    ///     原音のプロパティについて, which the page above points at for the entries
    class STDUTAU_EXPORT CharacterTxt {
    public:
        CharacterTxt();

        /// Opens \a path and reads it, returns \c false when the file will not open.
        bool load(const std::filesystem::path &path);

        /// Creates \a path and writes to it, returns \c false when the file will not open.
        bool save(const std::filesystem::path &path) const;

        /// Reads one \c key=value per line. A line that is not one goes to \a extraLines.
        bool read(std::string_view text);

        /// Writes the entries it has, then \a extraLines. An empty entry is left out, which is
        /// how the file says nothing rather than saying nothing in particular.
        std::string write() const;

    public:
        /// Shown in place of the folder name, which is what a bank without one is called.
        std::string name;

        /// Icon file, relative to the directory. UTAU wants a 100 by 100 bitmap.
        std::string image;

        /// Audio played by the Sample button. A bank without one plays whatever it finds.
        std::string sample;

        std::string author;
        std::string web;

        /// Lines this class has no entry for, kept as they were read and written back after the
        /// ones above.
        ///
        /// A \c character.txt is not only entries: UTAU shows a line holding a colon as part of
        /// the character's profile, which is why banks carry lines such as \c Version:1.0 . They
        /// are content, not noise, and dropping them would delete what the author wrote.
        std::vector<std::string> extraLines;
    };

}

#endif // CHARACTERTXT_H
