#ifndef PLUGINFILE_H
#define PLUGINFILE_H

#include <map>
#include <set>

#include <stdutau/ustfile.h>

namespace utau {

    /// Reads the temporary file that UTAU passes to a plugin at startup.
    ///
    /// UTAU passes the path as the first argument. The file contains the selected notes, plus
    /// the adjacent note on each side of the selection as context.
    ///
    /// The strings are raw bytes. They must be converted from the file encoding before use.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/64.html
    ///     プラグイン仕様, which defines this file and the permitted modifications
    class STDUTAU_EXPORT PluginFileReader {
    public:
        PluginFileReader();

        /// Reads the file at \a path . Returns \c false if it cannot be opened.
        bool load(const std::filesystem::path &path);

        /// \overload for file content already in memory.
        bool read(std::string_view text);

    public:
        /// Read-only, and not to be included in the output of the plugin.
        UstVersion version;
        UstSettings settings;

        /// The note before the selection, absent if the selection starts the track. A plugin
        /// edits it through PluginFileWriter::setPrevNote().
        std::optional<NoteExt> prevNote;

        /// The note after the selection, absent if the selection ends the track.
        std::optional<NoteExt> nextNote;

        /// The index of the first selected note within the entire track, taken from the number
        /// of the first numbered section. UTAU numbers the selected notes by their positions in
        /// the track, so a selection that starts at the third note starts at \c [#0002] .
        int startIndex;

        /// The selected notes, in track order.
        std::vector<NoteExt> notes;
    };

    /// Writes the modifications of a plugin over the temporary file it received.
    ///
    /// This class records edits rather than a complete track, and save() converts them into the
    /// sections UTAU expects. An unmodified note is written as unchanged, which instructs UTAU to
    /// keep its existing note.
    class STDUTAU_EXPORT PluginFileWriter {
    public:
        /// \a startIndex and \a originalSize are PluginFileReader::startIndex and the number of
        /// its notes. Every index below is a track index, counted like \a startIndex .
        PluginFileWriter(int startIndex, int originalSize);

        /// Writes to \a path , the temporary file passed to the plugin.
        bool save(const std::filesystem::path &path) const;

        /// \overload returning the bytes instead of writing a file.
        std::string write() const;

    public:
        /// Replaces the note at \a index with \a note .
        void setNote(int index, const Note &note);

        /// Replaces the note on either side of the selection, which UTAU permits a plugin to edit
        /// although it was passed as context.
        void setPrevNote(const Note &note);
        void setNextNote(const Note &note);

        /// Inserts \a notes before the note at \a index . An index one past the last selected
        /// note inserts after it.
        ///
        /// \note UTAU keeps insertions within the selection: a plugin cannot insert before the
        ///       previous note or after the next note.
        void insertNotes(int index, const std::vector<Note> &notes);

        /// Deletes the note at \a index . Indices are not renumbered, so consecutive calls require
        /// no index adjustment.
        void removeNote(int index);
        void removeNotes(const std::vector<int> &indexes);

    private:
        int m_startIndex;
        int m_originalSize;

        std::optional<Note> m_prevNote;
        std::optional<Note> m_nextNote;

        std::map<int, Note> m_changedNotes;
        std::map<int, std::vector<Note>> m_insertedNotes;
        std::set<int> m_removedNotes;
    };

}

#endif // PLUGINFILE_H
