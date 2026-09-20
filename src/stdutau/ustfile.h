#ifndef USTFILE_H
#define USTFILE_H

#include <array>
#include <map>
#include <string>
#include <optional>
#include <vector>
#include <filesystem>

#include <stdutau/utafilebase.h>
#include <stdutau/note.h>

namespace Utau {

    /// The \c [#VERSION] section.
    struct UstVersion {
        /// Format version, which is 1.2 in everything UTAU writes.
        std::string version;

        /// Name of the encoding the rest of the file is in, empty where the file does not say.
        /// Nothing in this library acts on it.
        std::string charset;
    };

    /// The \c [#SETTING] section. A project and a plugin temporary file share it, and a member
    /// says so where only one of the two fills it.
    class UstSettings {
    public:
        inline UstSettings();

    public:
        /// Tempo the track starts at, which a note may then change.
        double tempo;

        std::string flags;          ///< project only
        std::string projectName;    ///< project only
        std::string outputFileName; ///< project only

        /// Path of the project the plugin was called from.
        std::string project; ///< plugin only

        /// Voice bank directory, which UTAU writes with its \c %VOICE% prefix when the bank sits
        /// in the shared location.
        std::string voiceDir;

        /// Where rendered samples are kept. UTAU rewrites this to sit beside the saved file.
        std::string cacheDir;

        /// Paths of the two engines. **They come from the file, so treat them as untrusted.**
        /// UTAU running them as they stand is CVE-2024-28886.
        std::string wavtoolPath;
        std::string resamplerPath;

        /// Whether pitch is the mode 2 curve rather than the mode 1 sample array.
        bool isMode2;
    };

    inline UstSettings::UstSettings() : tempo(DEFAULT_VALUE_TEMPO), isMode2(false) {
    }

    /// A UTAU sequence text file, which is what a project is saved as.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    class STDUTAU_EXPORT UstFile : public UtaFileBase {
    public:
        UstFile();

        /// Reads the version, the settings and the notes. A section this library does not know is
        /// skipped, as is a note whose length is not positive.
        ///
        /// \warning UTAU turns a section it does not know into a note instead of skipping it,
        ///          which shifts every note index after that point. A file carrying one has
        ///          already been damaged, so do not read it as though the section were still
        ///          there.
        bool read(std::istream &is) override;

        /// Writes the version, the settings, the notes and the closing \c [#TRACKEND] .
        bool write(std::ostream &os) const override;

    public:
        UstVersion version;
        UstSettings settings;
        std::vector<Note> notes;
    };

}

#endif // USTFILE_H
