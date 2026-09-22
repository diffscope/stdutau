#ifndef PLUGINTXT_H
#define PLUGINTXT_H

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <stdutau/utaglobal.h>

namespace utau {

    /// The \c plugin.txt of a plugin folder, which describes the plugin before it runs.
    ///
    /// Distinct from PluginFileReader, which reads the temporary file UTAU passes to a running
    /// plugin. This class represents the manifest, PluginFileReader the payload.
    ///
    /// The strings are raw bytes. They must be converted from the file encoding before use.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様, which defines the entries below
    class STDUTAU_EXPORT PluginTxt {
    public:
        PluginTxt();

        /// Opens and reads \a path . Returns \c false if the file cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Creates and writes \a path . Returns \c false if the file cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// Reads one \c key=value pair per line. Any other line is stored in \a extraLines .
        bool read(std::string_view text);

        std::string write() const;

    public:
        /// The name of the menu entry.
        std::string name;

        /// The program to execute, relative to the plugin folder.
        ///
        /// \warning A path from a file on disk, and therefore untrusted.
        std::string execute;

        /// The \c shell entry. The value \c use means that UTAU starts the plugin with
        /// \c ShellExecuteEx rather than \c CreateProcess , which allows plugins that are not
        /// executables, such as jar, html and hta files.
        ///
        /// \warning \c ShellExecuteEx passes the file to the handler registered for its type, so
        ///          the program that actually runs is not specified here. It is not merely another
        ///          way of starting a process.
        std::string shell;

        /// The entry format of the temporary file. Empty if the file omits it, in which case the
        /// UTAU setting applies.
        ///
        /// UTAU 0.4.15 and later accept \c 1.00 ; \c 1.10 , which passes the mode 1 pitch array
        /// as \c Pitches ; and \c 1.20 , which passes it as \c PitchBend and names modulation
        /// \c Modulation .
        std::string ustVersion;

        /// The \c notes entry, absent if the file omits it.
        ///
        /// \note If present, the entire track is passed instead of the selection, regardless of
        ///       the value. This is why the member is an optional rather than a comparison with
        ///       \c all , which is merely the value used in the UTAU example.
        std::optional<std::string> notes;

        /// Lines without a dedicated member, preserved as read and written after the entries
        /// above.
        std::vector<std::string> extraLines;
    };

}

#endif // PLUGINTXT_H
