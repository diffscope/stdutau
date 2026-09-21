#ifndef OTOENTRY_H
#define OTOENTRY_H

#include <string>

#include <stdutau/utaglobal.h>

namespace utau {

    /// One entry of a voice bank's \c oto.ini, telling UTAU how to cut a sample.
    ///
    /// Every time here is in milliseconds and reaches the resampler as it stands. This class does
    /// not interpret them.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/106.html
    ///     原音設定, which is what the entries below are for
    class OtoEntry {
    public:
        inline OtoEntry();

        /// Sample file the entry describes, relative to the directory holding the \c oto.ini .
        std::string fileName;

        /// Name a lyric is matched against, which need not be the file name.
        std::string alias;

        /// Where the usable part of the sample begins, measured from the start of the file.
        double offset;

        /// Length of the part that must not be stretched, measured from \a offset.
        double consonant;

        /// Where the usable part ends. Positive measures back from the end of the file, negative
        /// gives the length from \a offset instead.
        double cutoff;

        /// How far ahead of its position on the track the note starts sounding.
        double preUtterance;

        /// How far the note reaches back into the one before it.
        double voiceOverlap;
    };

    inline OtoEntry::OtoEntry() {
        offset = 0.0;
        consonant = 0.0;
        cutoff = 0.0;
        preUtterance = 0.0;
        voiceOverlap = 0.0;
    }

}

#endif // OTOENTRY_H
