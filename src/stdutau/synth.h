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

    /// One call to the resampler, which renders a single note into a cache file.
    ///
    /// Fill the members, then hand arguments() to the process. Build the command line from that
    /// vector rather than from a string, which keeps a path holding a space or a quote from
    /// becoming further arguments.
    class STDUTAU_EXPORT ResamplerArguments {
    public:
        inline ResamplerArguments();

        /// The volume, the modulation and the pitch curve, which are what arguments() ends with.
        /// Call that instead unless you are assembling a command line by hand.
        std::vector<std::string> trailingArguments() const;

        /// The whole argument vector, in the order the resampler expects.
        std::vector<std::string> arguments() const;

    public:
        /// Position of the note in the track, which is how a caller rendering in parallel matches
        /// a finished job back to its note.
        int sequence;

        /// The sample to read and the cache file to write.
        std::string inFile;
        std::string outFile;

        /// Key to render at, as a tone name such as \c C4 .
        std::string toneName;

        double velocity;
        std::string flags;
        double intensity;
        double modulation;

        /// How long the rendered sample has to be, after the timing corrections below.
        double realLength;

        /// Taken from the OtoEntry and passed on unchanged. The resampler calls the last one the
        /// end blank, which is why it keeps that name here and not OtoEntry::cutoff.
        double offset;
        double consonant;
        double blank;

        /// Timing after Synth::calc() has settled what the note and its neighbours can each have.
        /// A note asking for more overlap than the one before it can give does not get it.
        double correctPreUttr;
        double correctOverlap;
        double correctStp;

        /// Pitch as one reading per sample, in cents. NODEF_INT marks a sample with no reading.
        std::vector<int> pitchCurves;
        double tempo;

        /// Whether the pitch curve goes out base64 encoded, which is what every current engine
        /// expects. Clear it only for an engine old enough to want the numbers spelled out.
        bool toBase64;
    };

    inline ResamplerArguments::ResamplerArguments()
        : sequence(0), velocity(100), intensity(100), modulation(0), realLength(0), offset(0),
          consonant(0), blank(0), correctPreUttr(0), correctOverlap(0), correctStp(0),
          tempo(DEFAULT_VALUE_TEMPO), toBase64(true) {
    }

    // wavtool2 <outfile> <infile> offset length p1 p2 p3 v1 v2 v3 v4 ovr p4 p5 v5

    /// One call to the wavtool, which appends a rendered note to the track being built.
    ///
    /// Same advice as for ResamplerArguments: hand arguments() to the process as a vector, not as
    /// a command line.
    class STDUTAU_EXPORT WavtoolArguments {
    public:
        inline WavtoolArguments();

        /// The length argument, which packs the length, the tempo and the correction into the one
        /// string the wavtool reads.
        std::string outDuration() const;

        /// The arguments carrying the envelope. A rest has none, so it gets two zeros instead.
        std::vector<std::string> envelopeArguments() const;

        /// The whole argument vector, in the order the wavtool expects.
        std::vector<std::string> arguments() const;

    public:
        /// The cache file to append and the track file being built up.
        std::string inFile;
        std::string outFile;

        double startPoint;
        double voiceOverlap;

        double tempo;

        /// Length of the note in ticks, where 480 is a quarter note.
        int length;

        /// How far the note moves from where its length alone would put it, once the preceding
        /// note's timing is settled.
        double correction;

        std::vector<Point> envelope;

        /// A rest, which contributes silence and no envelope.
        bool rest;
    };

    inline WavtoolArguments::WavtoolArguments()
        : startPoint(0), voiceOverlap(0), tempo(DEFAULT_VALUE_TEMPO), length(480), correction(0),
          rest(false) {
    }

    /// Turns a stretch of notes into the engine calls that render it.
    struct STDUTAU_EXPORT Synth {
        /// Returns the note at a track index. calc() asks for indexes just outside \a range as
        /// well, since a note's timing depends on its neighbours.
        using NoteGetter = std::function<Note(int)>;

        /// Returns the \c oto.ini entry a note resolves to, prefix map and alias lookup included.
        using OtoEntryGetter = std::function<OtoEntry(const Note &)>;

        /// One resampler call and one wavtool call per note, in track order.
        using SynthParams = std::vector<std::pair<ResamplerArguments, WavtoolArguments>>;

        /// Works out the engine calls for the notes in \a range.
        ///
        /// \a rangeLimits is what the track holds, and \a range the part to render. calc() reads
        /// one note beyond \a range on either side where \a rangeLimits allows, because overlap
        /// and pre-utterance are settled between neighbours.
        ///
        /// \a initialTempo applies until a note carries a tempo of its own. \a globalFlags are
        /// the project flags, and a note's own flags are appended to them.
        static SynthParams calc(const std::pair<int, int> &rangeLimits,
                                const std::pair<int, int> &range, double initialTempo,
                                const std::string &globalFlags, const NoteGetter &noteGetter,
                                const OtoEntryGetter &otoEntryGetter);
    };

}

#endif // SYNTH_H
