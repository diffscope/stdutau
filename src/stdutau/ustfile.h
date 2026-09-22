#ifndef USTFILE_H
#define USTFILE_H

#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/note.h>

namespace utau {

    /// The \c [#VERSION] section.
    struct UstVersion {
        /// The format version, which is 1.2 in every file UTAU writes.
        std::string version;

        /// The name of the encoding of the remaining file, empty if the file does not declare
        /// one. This library does not use it.
        std::string charset;
    };

    /// The \c [#SETTING] section, shared by projects and plugin temporary files. Members used by
    /// only one of the two are marked accordingly.
    class UstSettings {
    public:
        inline UstSettings();

    public:
        /// The initial tempo of the track, which a note may change.
        double tempo;

        std::string flags;          ///< project only
        std::string projectName;    ///< project only
        std::string outputFileName; ///< project only

        /// The path of the project from which the plugin was invoked.
        std::string project; ///< plugin only

        /// The voice bank directory, which UTAU writes with the \c %VOICE% prefix if the voice
        /// bank is in the shared directory.
        std::string voiceDir;

        /// The cache directory for rendered samples. UTAU updates it to match the saved file.
        std::string cacheDir;

        /// The paths of the two engines. **They come from the file and are therefore untrusted.**
        /// Executing them unchecked, as UTAU does, reproduces CVE-2024-28886.
        std::string wavtoolPath;
        std::string resamplerPath;

        /// Whether pitch uses the mode 2 curve rather than the mode 1 value array.
        bool isMode2;
    };

    inline UstSettings::UstSettings() : tempo(DEFAULT_VALUE_TEMPO), isMode2(false) {
    }

    /// A UTAU sequence text file, the format in which a project is saved.
    ///
    /// The strings are raw bytes. They must be converted from the file encoding before use.
    ///
    /// \note The official UTAU site publishes no format specification. The page below defines
    ///       the entries of the plugin temporary file, which are the entries of a UST.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様
    class STDUTAU_EXPORT UstFile {
    public:
        UstFile();

        /// Opens and reads \a path . Returns \c false if the file cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Creates and writes \a path . Returns \c false if the file cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// Reads the version, the settings and the notes. An unrecognized section is skipped, as
        /// is a note whose length is not positive.
        ///
        /// \warning UTAU converts an unrecognized section into a note instead of skipping it,
        ///          which shifts every subsequent note index. A file saved by UTAU with such a
        ///          section therefore no longer contains it.
        bool read(std::string_view text);

        /// Writes the version, the settings, the notes and the closing \c [#TRACKEND] .
        std::string write() const;

    public:
        UstVersion version;
        UstSettings settings;
        std::vector<Note> notes;
    };

}

#endif // USTFILE_H
