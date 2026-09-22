#ifndef SYNTH_H
#define SYNTH_H

#include <string>
#include <vector>
#include <functional>

#include <stdutau/otoentry.h>
#include <stdutau/note.h>

namespace utau {

    // resampler.exe <input wavfile> <output file> <pitch_percent> <velocity> [<flags> [<offset>
    // <length_require> [<fixed length> [<end_blank> [<volume> [<modulation> [<pich bend>...]]]]]]]

    /// One resampler call, which renders a single note into a cache file.
    ///
    /// Fill the members, then pass arguments() to the process as a vector rather than as a
    /// string, so that a path containing a space or a quotation mark cannot become additional
    /// arguments.
    class STDUTAU_EXPORT ResamplerArguments {
    public:
        inline ResamplerArguments();

        /// The volume, the modulation and the pitch curve, which form the end of arguments().
        /// Use arguments() unless a command line is assembled manually.
        std::vector<std::string> trailingArguments() const;

        /// The complete argument vector, in the order the resampler expects.
        std::vector<std::string> arguments() const;

    public:
        /// The position of the note in the track, by which a caller rendering in parallel matches
        /// a finished job to its note.
        int sequence;

        /// The sample to read and the cache file to write.
        std::string inFile;
        std::string outFile;

        /// The key to render at, as a tone name such as \c C4 .
        std::string toneName;

        double velocity;
        std::string flags;
        double intensity;
        double modulation;

        /// The required length of the rendered sample, after the timing corrections below.
        double realLength;

        /// Taken from the OtoEntry and passed unchanged. The resampler names the last value the
        /// end blank, which is why it is named so here rather than OtoEntry::cutoff.
        double offset;
        double consonant;
        double blank;

        /// The timing after Synth::calc() has reconciled the note with its neighbors. An overlap
        /// larger than the preceding note permits is reduced.
        double correctPreUttr;
        double correctOverlap;
        double correctStp;

        /// The pitch as one value per point, in cents. NODEF_INT marks a point without a value.
        std::vector<int> pitchCurves;
        double tempo;

        /// Whether the pitch curve is base64-encoded, as every current engine expects. Clear it
        /// only for an old engine that requires decimal numbers.
        bool toBase64;
    };

    inline ResamplerArguments::ResamplerArguments()
        : sequence(0), velocity(100), intensity(100), modulation(0), realLength(0), offset(0),
          consonant(0), blank(0), correctPreUttr(0), correctOverlap(0), correctStp(0),
          tempo(DEFAULT_VALUE_TEMPO), toBase64(true) {
    }

    // wavtool2 <outfile> <infile> offset length p1 p2 p3 v1 v2 v3 v4 ovr p4 p5 v5

    /// One wavtool call, which appends a rendered note to the track file.
    ///
    /// As with ResamplerArguments, pass arguments() to the process as a vector, not as a command
    /// line.
    class STDUTAU_EXPORT WavtoolArguments {
    public:
        inline WavtoolArguments();

        /// The length argument, which combines the length, the tempo and the correction into a
        /// single string.
        std::string outDuration() const;

        /// The envelope arguments. A rest has no envelope and receives two zeros instead.
        std::vector<std::string> envelopeArguments() const;

        /// The complete argument vector, in the order the wavtool expects.
        std::vector<std::string> arguments() const;

    public:
        /// The cache file to append and the track file being assembled.
        std::string inFile;
        std::string outFile;

        double startPoint;
        double voiceOverlap;

        double tempo;

        /// The length of the note in ticks, where 480 is a quarter note.
        int length;

        /// The timing correction relative to the position given by the length alone, once the
        /// timing of the preceding note is determined.
        double correction;

        std::vector<Point> envelope;

        /// Whether the note is a rest, which contributes silence and no envelope.
        bool rest;
    };

    inline WavtoolArguments::WavtoolArguments()
        : startPoint(0), voiceOverlap(0), tempo(DEFAULT_VALUE_TEMPO), length(480), correction(0),
          rest(false) {
    }

    /// Converts a range of notes into the engine calls that render it.
    struct STDUTAU_EXPORT Synth {
        /// Returns the note at a track index. calc() also requests indices just outside \a range ,
        /// because the timing of a note depends on its neighbors.
        using NoteGetter = std::function<Note(int)>;

        /// Returns the \c oto.ini entry of a note, including prefix map and alias lookup.
        using OtoEntryGetter = std::function<OtoEntry(const Note &)>;

        /// One resampler call and one wavtool call per note, in track order.
        using SynthParams = std::vector<std::pair<ResamplerArguments, WavtoolArguments>>;

        /// Computes the engine calls for the notes in \a range .
        ///
        /// \a rangeLimits is the extent of the track, and \a range the part to render. calc()
        /// reads one note beyond \a range on each side if \a rangeLimits permits, because overlap
        /// and pre-utterance are determined between neighbors.
        ///
        /// \a initialTempo applies until a note specifies its own tempo. \a globalFlags are the
        /// project flags, to which the flags of each note are appended.
        static SynthParams calc(const std::pair<int, int> &rangeLimits,
                                const std::pair<int, int> &range, double initialTempo,
                                const std::string &globalFlags, const NoteGetter &noteGetter,
                                const OtoEntryGetter &otoEntryGetter);
    };

}

#endif // SYNTH_H
