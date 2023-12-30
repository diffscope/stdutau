#ifndef SYNTH_H
#define SYNTH_H

#include <string>
#include <vector>
#include <functional>

#include <stdutau/genonsettings.h>
#include <stdutau/note.h>

namespace Utau {

    // resampler.exe <input wavfile> <output file> <pitch_percent> <velocity> [<flags> [<offset>
    // <length_require> [<fixed length> [<end_blank> [<volume> [<modulation> [<pich bend>...]]]]]]]

    class STDUTAU_EXPORT ResamplerArguments {
    public:
        inline ResamplerArguments();

        std::vector<std::string> params() const;
        std::vector<std::string> arguments() const;

    public:
        int sequence;

        std::string inFile;
        std::string outFile;
        std::string toneName;

        double velocity;

        std::string flags;

        double intensity;
        double modulation;
        double realLength;

        double offset;
        double consonant;
        double blank;
        double correctPreUttr;
        double correctOverlap;
        double correctStp;

        std::vector<int> pitchCurves;
        double tempo;

        bool toBase64;
    };

    inline ResamplerArguments::ResamplerArguments()
        : sequence(0), velocity(100), intensity(100), modulation(0), realLength(0), offset(0),
          consonant(0), blank(0), correctPreUttr(0), correctOverlap(0), correctStp(0),
          tempo(DEFAULT_VALUE_TEMPO), toBase64(true) {
    }

    // wavtool2 <outfile> <infile> offset length p1 p2 p3 v1 v2 v3 v4 ovr p4 p5 v5

    class STDUTAU_EXPORT WavtoolArguments {
    public:
        inline WavtoolArguments();

        std::string outDuration() const;
        std::vector<std::string> env() const;
        std::vector<std::string> arguments() const;

    public:
        std::string inFile;
        std::string outFile;

        double startPoint;
        double voiceOverlap;

        double tempo;
        int length;
        double correction;

        std::vector<Point> envelope;

        bool rest; // Is rest note?
    };

    inline WavtoolArguments::WavtoolArguments()
        : startPoint(0), voiceOverlap(0), tempo(DEFAULT_VALUE_TEMPO), length(480), correction(0),
          rest(false) {
    }

    struct STDUTAU_EXPORT Synth {
        using NoteGetter = std::function<Note(int)>;
        using GenonSettingsGetter = std::function<GenonSettings(const Note &)>;

        static void calc(const std::pair<int, int> &rangeLimits, const std::pair<int, int> &range,
                         const NoteGetter &noteGetter,
                         const GenonSettingsGetter &genonSettingsGetter,
                         std::vector<std::pair<ResamplerArguments, WavtoolArguments>> *result);
    };

}

#endif // SYNTH_H
