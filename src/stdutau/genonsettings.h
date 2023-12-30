#ifndef GENONSETTINGS_H
#define GENONSETTINGS_H

#include <string>

#include <stdutau/utaglobal.h>

namespace Utau {

    class GenonSettings {
    public:
        inline GenonSettings();

        std::string fileName;
        double offset;
        double cosonant;
        double blank;
        double preUtterance;
        double voiceOverlap;
    };

    inline GenonSettings::GenonSettings() {
        offset = 0.0;
        cosonant = 0.0;
        blank = 0.0;
        preUtterance = 0.0;
        voiceOverlap = 0.0;
    }

}

#endif // GENONSETTINGS_H
