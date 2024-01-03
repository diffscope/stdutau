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

    struct UstVersion {
        std::string version;
        std::string charset;
    };

    class UstSettings {
    public:
        inline UstSettings();

    public:
        double tempo;
        std::string flags;          // project only

        std::string projectName;    // project only
        std::string outputFileName; // project only

        std::string project;        // plugin only

        std::string voiceDir;
        std::string cacheDir;

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
        UstVersion version;
        UstSettings settings;
        std::vector<Note> notes;
    };

}

#endif // USTFILE_H
