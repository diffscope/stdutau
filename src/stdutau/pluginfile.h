#ifndef PLUGINFILE_H
#define PLUGINFILE_H

#include <map>
#include <set>

#include <stdutau/ustfile.h>

namespace utau {

    /// Reads the temporary file UTAU hands a plugin on startup.
    ///
    /// UTAU passes the path as the first argument. The file holds the selected notes, plus the
    /// one on either side of the selection for context.
    ///
    /// The strings here are raw bytes. Work out the encoding and convert before you look at them.
    class STDUTAU_EXPORT PluginFileReader {
    public:
        PluginFileReader();

        /// Reads the file at \a path, returns \c false when it will not open.
        bool load(const std::filesystem::path &path);

    public:
        /// Read only, and not wanted in what the plugin writes back.
        UstVersion version;
        UstSettings settings;

        /// The note before the selection, absent when the selection starts the track. It is
        /// context, so changing it is not how a plugin edits the track.
        std::optional<NoteExt> prevNote;

        /// The note after the selection, absent when the selection ends the track.
        std::optional<NoteExt> nextNote;

        /// Index of the first selected note within the whole track.
        ///
        /// \note The temporary file numbers its notes from zero whatever the selection is, so
        ///       nothing in the file says where the selection sits and load() leaves this at
        ///       zero. Set it from what the host knows before passing it to PluginFileWriter.
        int startIndex;

        /// The selected notes, in track order.
        std::vector<NoteExt> notes;
    };

    /// Writes back what a plugin changed, over the temporary file it was given.
    ///
    /// This records edits rather than a finished track, and save() turns them into the sections
    /// UTAU expects. A note left alone is written as unchanged, which is how UTAU is told to keep
    /// what it already has.
    class STDUTAU_EXPORT PluginFileWriter {
    public:
        /// \a startIndex and \a originalSize are PluginFileReader::startIndex and the size of its
        /// notes. Every index below is a track index, counted as \a startIndex is.
        PluginFileWriter(int startIndex, int originalSize);

        /// Writes to \a path, which is the file the plugin was given.
        bool save(const std::filesystem::path &path) const;

    public:
        /// Replaces the note at \a index with \a note.
        void setNote(int index, const Note &note);

        /// Replaces the note on either side of the selection, which UTAU allows a plugin to edit
        /// although it was passed as context.
        void setPrevNote(const Note &note);
        void setNextNote(const Note &note);

        /// Adds \a notes in front of the note at \a index.
        void insertNotes(int index, const std::vector<Note> &notes);

        /// Adds \a notes outside the selection, before the previous note or after the next one.
        void prependNotesBeforePrev(const std::vector<Note> &notes);
        void appendNotesAfterNext(const std::vector<Note> &notes);

        /// Deletes the note at \a index. Indexes stay as they were, so several calls need no
        /// arithmetic between them.
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

        std::vector<Note> m_notesBeforePrev;
        std::vector<Note> m_notesAfterNext;
    };

}

#endif // PLUGINFILE_H
