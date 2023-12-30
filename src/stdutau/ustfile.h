#ifndef USTFILE_H
#define USTFILE_H

#include <array>
#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include <stdutau/utafilebase.h>
#include <stdutau/note.h>

namespace Utau {

    struct UstVersion {
        std::string version;
        std::string charset;
    };

    class UstSettings {
    public:
        inline UstSettings();

    public:
        double tempo;
        std::string flags;

        std::string projectName;
        std::string outputFileName;

        std::string voiceDirectory;
        std::string cacheDirectory;

        std::string wavtoolPath;
        std::string resamplerPath;

        bool isMode2;
    };

    inline UstSettings::UstSettings() : tempo(DEFAULT_VALUE_TEMPO), isMode2(false) {
    }

    class STDUTAU_EXPORT UstFile : public UtaFileBase {
    public:
        UstFile();

        bool read(std::istream &is) override;
        bool write(std::ostream &os) const override;

    public:
        UstVersion sectionVersion;
        UstSettings sectionSettings;
        std::vector<Note> sectionNotes;
    };

}

#endif // USTFILE_H
