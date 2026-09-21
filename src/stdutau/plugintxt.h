#ifndef PLUGINTXT_H
#define PLUGINTXT_H

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// A plugin folder's \c plugin.txt , which is what UTAU knows about a plugin before running
    /// it.
    ///
    /// Not to be confused with PluginFileReader, which reads the temporary file UTAU hands the
    /// plugin once it runs. This is the manifest, that is the payload.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様, which is where the entries below are defined
    class STDUTAU_EXPORT PluginTxt {
    public:
        PluginTxt();

        /// Opens \a path and reads it, returns \c false when the file will not open.
        bool load(const std::filesystem::path &path);

        /// Creates \a path and writes to it, returns \c false when the file will not open.
        bool save(const std::filesystem::path &path) const;

        /// Reads one \c key=value per line. A line that is not one goes to \a extraLines.
        bool read(std::string_view text);

        std::string write() const;

    public:
        /// What the menu entry is called.
        std::string name;

        /// The program to run, relative to the plugin's folder.
        ///
        /// \warning This is a path out of a file on disk, so treat it as untrusted.
        std::string execute;

        /// The \c shell entry. The value \c use means UTAU starts the plugin with
        /// \c ShellExecuteEx rather than \c CreateProcess , which is how a plugin that is not an
        /// executable runs at all: a jar, an html, an hta.
        ///
        /// \warning \c ShellExecuteEx hands the file to whatever the system registered for its
        ///          type, so what actually runs is not named here. Do not treat it as merely
        ///          another way to start a process.
        std::string shell;

        /// Which entries the temporary file is written with, empty where the file leaves it out
        /// and UTAU's own setting decides.
        ///
        /// UTAU 0.4.15 and later accept \c 1.00 , \c 1.10 , which passes the mode 1 pitch array
        /// as \c Pitches , and \c 1.20 , which passes it as \c PitchBend and spells modulation
        /// \c Modulation .
        std::string ustVersion;

        /// The \c notes entry, absent where the file leaves it out.
        ///
        /// \note Present at all means the whole track is handed over rather than the selection,
        ///       whatever the value. That is why this is an optional and not a comparison
        ///       against \c all , which is only what UTAU's own example writes.
        std::optional<std::string> notes;

        /// Lines this class has no entry for, kept as they were read and written back after the
        /// ones above.
        std::vector<std::string> extraLines;
    };

}

#endif // PLUGINTXT_H
