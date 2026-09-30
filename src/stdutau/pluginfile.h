#ifndef PLUGINFILE_H
#define PLUGINFILE_H

#include <map>
#include <set>

#include <stdutau/ustfile.h>

namespace utau {

    /// The temporary file that a host passes to a plugin at startup: a plugin reads it, and a
    /// host writes it as UTAU does.
    ///
    /// UTAU passes the path as the first argument. The file contains the selected notes, plus
    /// the adjacent note on each side of the selection as context. It declares
    /// \c UST \c Version \c 1.20 , and its lines end with CRLF.
    ///
    /// The strings are raw bytes, in the encoding of the file.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様, which defines this file and the permitted modifications
    class STDUTAU_EXPORT PluginInput {
    public:
        PluginInput();

        /// Reads the file at \a path . Returns \c false if it cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Writes to \a path . Returns \c false if it cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// \overload for file content already in memory.
        bool read(std::string_view text);

        /// \overload returning the bytes instead of writing a file.
        std::string write() const;

    public:
        /// As read from the file. write() always declares 1.20, the format of the entries that
        /// it writes.
        UstVersion version;

        /// Of the settings, the file carries UstSettings::project, the tempo at the start of
        /// the selection, the voice and cache directories and Mode2. UTAU writes the paths
        /// absolute. The settings are read-only, and not to be included in the output of the
        /// plugin.
        UstSettings settings;

        /// The note before the selection, absent if the selection starts the track or the
        /// plugin receives the whole track. A plugin edits it through a Prev section of its
        /// PluginResult.
        std::optional<NoteExt> prevNote;

        /// The note after the selection, absent if the selection ends the track or the plugin
        /// receives the whole track.
        std::optional<NoteExt> nextNote;

        /// The index of the first selected note within the entire track, which numbers the
        /// sections. UTAU numbers the selected notes by their positions in the track, so a
        /// selection that starts at the third note starts at \c [#0002] .
        int startIndex;

        /// The selected notes in track order, or every note if the plugin asks for the whole
        /// track in PluginTxt::notes.
        std::vector<NoteExt> notes;
    };

    /// The temporary file that a plugin writes back over its PluginInput: the sections to apply
    /// to the selection, in their order. A plugin writes it, and a host reads it.
    ///
    /// UTAU applies the sections in their order in the file, not by their numbers. An entry
    /// that a section omits leaves the note unchanged, and an entry with an empty value restores
    /// the default. A file without a section of a note cancels the plugin.
    ///
    /// \code
    ///   PluginInput input;
    ///   input.load(path);
    ///   PluginResult result(input);           // every note unchanged
    ///   auto note = input.notes[0];
    ///   note.lyric = "la";
    ///   result.sections[0].assign(note);      // the first note changed
    ///   result.sections.insert(result.sections.begin() + 1,
    ///                          PluginResult::Section(PluginResult::Section::Delete));
    ///   result.save(path);                    // ... and the second deleted
    /// \endcode
    ///
    /// \note UTAU keeps insertions within the selection: an Insert section before the Prev
    ///       section inserts at the start of the selection, one after the Next section at its
    ///       end.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様, which defines the permitted modifications
    class STDUTAU_EXPORT PluginResult {
    public:
        /// One section of a note.
        struct STDUTAU_EXPORT Section {
            enum Kind {
                Numbered, ///< the next note of the selection
                Insert,   ///< a new note at this position
                Delete,   ///< removes the next note of the selection
                Prev,     ///< the note before the selection
                Next,     ///< the note after the selection
            };

            /// A section without entries, which leaves a note unchanged, and a blank note.
            Section(Kind kind = Numbered, int number = 0);

            /// Sets the note to \a note and \a keys to the entries that \a note has. The
            /// entries that it lacks stay unchanged in UTAU.
            void assign(const Note &note);

            Kind kind;

            /// The number of a Numbered section, which UTAU does not use.
            int number;

            /// The names of the entries in the section, under the names that Note reads them
            /// under: \c Moduration as \c Modulation , \c Piches and \c Pitches as
            /// \c PitchBend . The read-only \c @ entries and \c PBType are left out. A Delete
            /// section has none.
            std::set<std::string> keys;

            /// The values of the entries in \a keys . An entry with an empty value leaves its
            /// member blank: absent, empty or zero, and a blank member of an entry in \a keys is
            /// written as an empty value. Members of other entries have no meaning.
            Note note;
        };

        PluginResult();

        /// A Numbered section without entries for each note of \a input , numbered from its
        /// start: the result that changes nothing.
        explicit PluginResult(const PluginInput &input);

        /// Reads the file at \a path . Returns \c false if it cannot be opened.
        bool load(const std::filesystem::path &path);

        /// Writes to \a path . Returns \c false if it cannot be opened.
        bool save(const std::filesystem::path &path) const;

        /// \overload for file content already in memory.
        bool read(std::string_view text);

        /// \overload returning the bytes instead of writing a file.
        std::string write() const;

        /// Whether there is no section of a note, which cancels the plugin.
        bool isCancelled() const;

    public:
        /// The sections of notes in their order. The version and the settings of the file are
        /// read-only, and neither read nor written. A section of another name is skipped.
        std::vector<Section> sections;
    };

}

#endif // PLUGINFILE_H
