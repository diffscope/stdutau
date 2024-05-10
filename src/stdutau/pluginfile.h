#ifndef PLUGINFILE_H
#define PLUGINFILE_H

#include <memory>

#include <stdutau/ustfile.h>

namespace Utau {

    class STDUTAU_EXPORT PluginFileReader {
    public:
        PluginFileReader();

        bool load(const std::filesystem::path &path);

    public:
        UstVersion version() const;
        UstSettings settings() const;

        std::optional<NoteExt> prevNote() const;
        std::optional<NoteExt> nextNote() const;

        int startIndex() const;
        std::vector<NoteExt> notes() const;

    protected:
        struct Private;
        std::shared_ptr<Private> d_ptr;
    };

    class STDUTAU_EXPORT PluginFileWriter {
    public:
        PluginFileWriter(int startIndex, int originalSize);

        bool save(const std::filesystem::path &path) const;

    public:
        void setNote(int index, const Note &note);
        void setPrevNote(const Note &note);
        void setNextNote(const Note &note);

        void insertNotes(int index, const std::vector<Note> &notes);
        void prependNotesBeforePrev(const std::vector<Note> &notes);
        void appendNotesAfterNext(const std::vector<Note> &notes);

        void removeNote(int index);
        void removeNotes(const std::vector<int> &indexes);

    protected:
        struct Private;
        std::shared_ptr<Private> d_ptr;
    };

}

#endif // PLUGINFILE_H
