#ifndef OTOENTRY_H
#define OTOENTRY_H

#include <string>

#include <stdutau/utaglobal.h>

namespace utau {

    /// One entry of the \c oto.ini of a voice bank, which specifies how UTAU cuts a sample.
    ///
    /// All times are in milliseconds and are passed to the resampler unchanged. This class does
    /// not interpret them.
    ///
    /// \sa https://w.atwiki.jp/utaou/pages/106.html
    ///     原音設定, which defines the entries below
    class OtoEntry {
    public:
        inline OtoEntry();

        /// The sample file of the entry, relative to the directory containing the \c oto.ini .
        std::string fileName;

        /// The name against which a lyric is matched, which may differ from the file name.
        std::string alias;

        /// The start of the usable part of the sample, measured from the start of the file.
        double offset;

        /// The length of the part that must not be stretched, measured from \a offset .
        double consonant;

        /// The end of the usable part. A positive value is measured backward from the end of the
        /// file, a negative value specifies the length from \a offset instead.
        double cutoff;

        /// The time by which the note starts sounding before its position on the track.
        double preUtterance;

        /// The time by which the note overlaps the preceding note.
        double voiceOverlap;

        /// The original text of the five numbers above, in declaration order.
        ///
        /// A voice bank may write the same value in different forms, often within one file and
        /// sometimes on two lines of the same sample: \c 41 on one, \c 41.0 on the next. No
        /// single formatting rule reproduces both, so each number retains its original text, and
        /// an unmodified file is saved unchanged.
        ///
        /// The original text is written only if it still parses to exactly the current value, so
        /// a modified number is formatted anew without explicit invalidation. Empty for an entry
        /// that was not read from a file.
        std::string spellings[5];
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
