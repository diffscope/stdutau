#ifndef GENONSETTINGS_H
#define GENONSETTINGS_H

#include <string>

#include <stdutau/utaglobal.h>

namespace Utau {

    /// One entry of a voice bank's \c oto.ini, telling UTAU how to cut a sample.
    ///
    /// Every time here is in milliseconds and reaches the resampler as it stands. This class does
    /// not interpret them.
    ///
    /// \note The name follows \c TGenonSettings in SHINTA's LibUtau. \c genon is the Japanese
    ///       for the original recorded voice, 原音.
    class GenonSettings {
    public:
        inline GenonSettings();

        /// Sample file the entry describes, relative to the directory holding the \c oto.ini .
        std::string fileName;

        /// Name a lyric is matched against, which need not be the file name.
        std::string alias;

        /// Where the usable part of the sample begins, measured from the start of the file.
        double offset;

        /// Length of the part that must not be stretched, measured from \a offset.
        double consonant;

        /// Where the usable part ends. \c oto.ini gives this one two readings depending on its
        /// sign, and neither this class nor \c Synth picks between them.
        double blank;

        /// How far ahead of its position on the track the note starts sounding.
        double preUtterance;

        /// How far the note reaches back into the one before it.
        double voiceOverlap;
    };

    inline GenonSettings::GenonSettings() {
        offset = 0.0;
        consonant = 0.0;
        blank = 0.0;
        preUtterance = 0.0;
        voiceOverlap = 0.0;
    }

}

#endif // GENONSETTINGS_H
