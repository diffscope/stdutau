#ifndef GENONSETTINGS_H
#define GENONSETTINGS_H

#include <string>

#include <stdutau/utaglobal.h>

namespace Utau {

    class STDUTAU_EXPORT GenonSettings {
    public:
        GenonSettings();
        ~GenonSettings();

        std::string fileName;
        double offset;
        double cosonant;
        double blank;
        double preUtterance;
        double voiceOverlap;
    };

}

#endif // GENONSETTINGS_H
