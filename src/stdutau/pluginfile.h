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

    public:
        struct Private;
        std::unique_ptr<Private> d_ptr;
    };

    class STDUTAU_EXPORT PluginFileWriter {
    public:
        PluginFileWriter();

        bool save(const std::filesystem::path &path) const;

    public:
        void setChangedPrevNote(const Note &note);
        void setChangedNextNote(const Note &note);

        void addChangedNote(int index, const Note &note);
        void addChangedNotes(int index, const std::vector<Note> &notes);

        void addInsertedNote(int index, const Note &note);
        void addInsertedNotes(int index, const std::vector<Note> &notes);

        void addRemovedNote(int index);
        void addRemovedNotes(const std::vector<int> &indexes);

    protected:
        struct Private;
        std::unique_ptr<Private> d_ptr;
    };

}

#endif // PLUGINFILE_H
