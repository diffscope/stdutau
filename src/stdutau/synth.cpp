#include "synth.h"

#include <cmath>

#include "utautils.h"

#define PI 3.1415926

namespace Utau {

    static inline std::vector<std::string> &operator<<(std::vector<std::string> &vec,
                                                       const std::string &item) {
        vec.push_back(item);
        return vec;
    }

    static inline std::vector<std::string> &operator<<(std::vector<std::string> &vec,
                                                       const std::vector<std::string> &vec2) {
        for (const auto &item : vec2)
            vec.push_back(item);
        return vec;
    }

    //
    // Port from QSynthesis begin
    //

    namespace UtaPitchCurves {

        static const char Base64EncodeMap[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

        static inline double f_x(const double &x1, const double &y1, const double &x2,
                                 const double &y2, const double &x) {
            if (x1 == x2) {
                return y1;
            }
            return (y1 - y2) / 2 * std::cos(PI * (x - x1) / (x2 - x1)) + (y1 + y2) / 2;
        }


        static inline double f_r(const double &x1, const double &y1, const double &x2,
                                 const double &y2, const double &x) {
            if (x1 == x2) {
                return y1;
            }
            return (y2 - y1) * std::cos(PI / 2 / (x2 - x1) * (x - x2)) + y1;
        }


        static inline double f_j(const double &x1, const double &y1, const double &x2,
                                 const double &y2, const double &x) {
            if (x1 == x2) {
                return y1;
            }
            return (y1 - y2) * std::cos(PI / 2 / (x2 - x1) * (x - x1)) + y2;
        }


        static inline double f_s(const double &x1, const double &y1, const double &x2,
                                 const double &y2, const double &x) {
            if (x1 == x2) {
                return y1;
            }
            return (y2 - y1) / (x2 - x1) * (x - x1) + y1;
        }

        static inline double f_type(const Point::Type &ptype, const double &x1, const double &y1,
                                    const double &x2, const double &y2, const double &x) {
            int impact;
            switch (ptype) {
                case Point::linearJoin:
                    impact = int(f_s(x1, y1, x2, y2, x));
                    break;
                case Point::jJoin:
                    impact = int(f_j(x1, y1, x2, y2, x));
                    break;
                case Point::rJoin:
                    impact = int(f_r(x1, y1, x2, y2, x));
                    break;
                default:
                    impact = int(f_x(x1, y1, x2, y2, x));
                    break;
            }
            return impact;
        }

        static int find_impact(const std::vector<Point> &portamento, int &startIndex, int curTick,
                               const double &PositiveTempo, double NegativeTempo,
                               const std::vector<double> &vibrato, const int &length) {

            // portamento: Mode2 Pitch curve points
            // startIndex: search from index
            // curTick: current tick
            // PositiveTempo: tempo used when ticks is positive
            // NegativeTempo: tempo used when ticks is negative (necessary when tempo changed)
            // vibrato: The vibrato sequence, which can be absent

            if (startIndex < 0) {
                return 0;
            }

            int basePitch;
            int &i = startIndex; // Self counter
            double x1, y1, x2, y2;
            bool tooLeft;

            basePitch = 0;

            // Search portatmento
            if (!portamento.empty()) {
                tooLeft = false;

                while (i < portamento.size() - 1) {
                    // Last point
                    x1 = portamento[i].x * ((portamento[i].x < 0) ? NegativeTempo : PositiveTempo) /
                         60 * 480 / 1000;

                    if (curTick < x1) {
                        if (i > 0) {
                            i--; // startIndex is incorrect, need to search forward
                            continue;
                        } else if (i == 0) {
                            tooLeft = true;
                            break;
                        }
                    }

                    y1 = portamento[i].y * 10;

                    // Next point
                    x2 = portamento[i + 1].x *
                         ((portamento[i + 1].x < 0) ? NegativeTempo : PositiveTempo) / 60 * 480 /
                         1000;

                    y2 = portamento[i + 1].y * 10;

                    auto ptype = portamento[i + 1].type;

                    if (curTick > x2) {
                        i++;
                        continue;
                    }
                    basePitch = f_type(ptype, x1, y1, x2, y2, curTick);
                    break;
                }
                if (tooLeft) {
                    basePitch = int(portamento[0].y * 10); // haven't reached the first point yet
                }
            }

            // Search vibrato
            if (vibrato.size() >= 8 && length > 0) {
                double proportion = vibrato[0];
                double period = vibrato[1];
                double amplitude = vibrato[2];
                double easeIn = vibrato[3];
                double easeOut = vibrato[4];
                double phase = vibrato[5];
                double offset = vibrato[6];

                proportion /= 100.0;

                double tick_time = period * PositiveTempo / 60 * 480 / 1000;
                double tick_length = proportion * length;
                double tick_start = (1 - proportion) * length; // ticks time relative to start

                double ratio;
                double x, k, p, y;

                k = 1 / tick_time * 2 * PI;                    // Circular frequency
                p = phase / 100.0 * 2 * PI;                    // Initial phase

                easeIn = easeIn / 100.0 * tick_length;         // Fade in time
                easeOut = (1 - easeOut / 100.0) * tick_length; // Fade out time
                offset /= 100.0;

                x = curTick - tick_start;       // tick x
                y = amplitude * sin(k * x - p); // original vibrato

                if (x > 0 && x < tick_length) {
                    ratio = 1;
                    // Add offset
                    y += offset * amplitude;
                    // Calculate envelope
                    if (x < easeIn) {
                        ratio *= x / easeIn;
                    }
                    if (x > easeOut) {
                        ratio *= 1 - (x - easeOut) / (tick_length - easeOut);
                    }
                    // Add envelope
                    y = ratio * y;
                    // Add influence
                    basePitch += y;
                }
            }

            return basePitch;
        }

        static std::vector<int> convert_from_vector_point(
            const double &tempo1, const std::vector<Point> &curNote,
            const std::vector<double> &curVBR, const double &curPre, const double &curStp,
            const int &curLength, const std::vector<Point> &nextNote,
            const std::vector<double> &nextVBR, const double &nextPre, const double &nextOve,
            const int &nextLength, const std::vector<Point> &prevNote,
            const std::vector<double> &prevVBR, const int &prevLength) {

            // Mode 2 to Mode 1 principle
            // 1. Pre-Utterance part, use the previous note tempo (actually not)
            // 2. For the rest part, use its own tempo
            // 3. Pre-Utterance may be affected by the pitch line of the preceding note
            // 4. The rest part may be affected by the pitch line of the next note

            // curNote saves its own control point, nextNote saves the control point of the
            // nextNote, prevNote saves the control point of the previous note tempo stores its own
            // tempo, tempo2 stores the previous note's tempo

            std::vector<int> PitchBend;
            double duration, nextStart, pbstart;
            int basePitch, prevImpact, nextImpact;
            int i, j, k; // Self, successor, precursor counter

            double tick;

            double curTempo = tempo1;
            double prevTempo = tempo1;

            // Ignore the length of the pre-utterance
            duration = double(curLength) + (-nextPre + nextOve) * curTempo / 60 * 480 / 1000;
            // The next note starts the effect
            nextStart = (!nextNote.empty())
                            ? (curLength + nextNote[0].x * curTempo / 60 * 480 / 1000)
                            : INFINITY;

            i = 0;
            j = 0;
            k = 0;

            // Set the starting point on the previous note (- PBStart = STP + pre)
            pbstart = -(curPre + curStp) * prevTempo / 60 * 480 / 1000;
            tick = pbstart;

            while (tick < duration) {
                prevImpact = 0;
                nextImpact = 0;

                basePitch = find_impact(curNote, i, tick, curTempo, prevTempo, curVBR, curLength);

                // The part influenced by the next note
                if (tick >= nextStart) {
                    if (j < nextNote.size() - 1) {
                        nextImpact = find_impact(nextNote, j, tick - curLength, curTempo, curTempo,
                                                 nextVBR, nextLength);
                    }
                    nextImpact += -nextNote[0].y * 10;
                }

                // The part influenced by the previous note
                if (tick <= 0) {
                    prevImpact = find_impact(prevNote, k, tick + prevLength, prevTempo, prevTempo,
                                             prevVBR, prevLength);
                }

                // Add the influence of the pitch line before and after the note
                PitchBend.push_back(basePitch + prevImpact + nextImpact);
                tick = tick + 5;
            }

            // Delete the redundant 0
            while (PitchBend.size() >= 7 && PitchBend.back() == 0) {
                PitchBend.pop_back();
            }

            return PitchBend;
        }

        static std::string encode_single_num(int n) {
            // If the value is negative, the 12-bit binary is inverted
            if (n < 0) {
                n += 4096;
            }

            char x = Base64EncodeMap[int(n / 64)];
            char y = Base64EncodeMap[n % 64];

            std::string result;
            result.push_back(x);
            result.push_back(y);
            return result;
        }

        static std::string encode_from_vector(const std::vector<int> &pitchBend) {
            int pos = 0;
            int count = 0;
            int curInt;
            int prevInt = INT32_MIN;
            std::string result;

            while (pos < pitchBend.size()) {
                pos++;
                curInt = (pitchBend[pos - 1] == NODEF_INT) ? 0 : pitchBend[pos - 1];

                if (curInt == prevInt) {
                    ++count;
                    // Final process
                    if (pos == pitchBend.size()) {
                        if (count >= 2) {
                            result += "#" + std::to_string(count) + "#";
                        } else {
                            result += encode_single_num(prevInt);
                        }
                    }
                } else {
                    if (count != 0) {
                        // Use n-1 to replace the rest when appear repeatedly
                        if (count >= 2) {
                            result += "#" + std::to_string(count) + "#";
                        } else {
                            result += encode_single_num(prevInt);
                        }
                        count = 0;
                    }
                    result += encode_single_num(curInt);
                }

                prevInt = curInt;
            }

            return result;
        }

    }

    namespace UtaTranslator {

        static std::vector<std::string> EnvelopeToStringList(const std::vector<Point> &tpoints,
                                                             const double &overlap) {
            std::string strOverlap = std::to_string(overlap);
            std::vector<std::string> listEnv;

            if (tpoints.size() < 4) {
                listEnv << "0"
                        << "5"
                        << "35"
                        << "0"
                        << "100"
                        << "100"
                        << "0" << strOverlap;
                return listEnv;
            }

            listEnv << std::to_string(tpoints.at(0).x);
            listEnv << std::to_string(tpoints.at(1).x);
            listEnv << std::to_string(tpoints.at(tpoints.size() - 2).x);
            listEnv << std::to_string(tpoints.at(0).y);
            listEnv << std::to_string(tpoints.at(1).y);
            listEnv << std::to_string(tpoints.at(tpoints.size() - 2).y);
            listEnv << std::to_string(tpoints.at(tpoints.size() - 1).y);
            listEnv << strOverlap;
            if (tpoints.size() == 5) {
                listEnv << std::to_string(tpoints.at(tpoints.size() - 1).x);
                listEnv << std::to_string(tpoints.at(2).x);
                listEnv << std::to_string(tpoints.at(2).y);
            } else if (tpoints.at(tpoints.size() - 1).x != 0) {
                listEnv << std::to_string(tpoints.at(tpoints.size() - 1).x);
            }
            return listEnv;
        }

        static inline void getCorrectPBSY(int prevNoteNum, const std::string &prevLyric,
                                          int curNoteNum, Point &curPoint) {
            if (!isRestLyric(prevLyric)) {
                double y1 = (prevNoteNum <= 0) ? 0 : double((prevNoteNum - curNoteNum) * 10);
                curPoint.y = y1;
            }
        }

        static std::vector<Point> getDefaultPitch(const int &prevNoteNum,
                                                  const std::string &prevLyric,
                                                  const int &curNoteNum) {

            Point first(0, 0);
            Point second(0, 0);

            getCorrectPBSY(prevNoteNum, prevLyric, curNoteNum, first);
            second.type = Point::sJoin;

            std::vector<Point> pitch = {first, second};

            return pitch;
        }

        static std::string fixFlags(const std::string &s) {
            std::string s2 = "";

            for (std::string::size_type i = 0; i < s.size(); ++i) {
                auto ch = s.at(i);
                if (ch == '\"') {
                    continue;
                }
                if (ch == 'e' || ch == 'E') {
                    s2 += '/';
                }
                s2 += ch;
            }

            return s2;
        }

        static std::string fixFilename(const std::string &filename) {
            std::string s;

            for (std::string::size_type i = 0; i < filename.size(); ++i) {
                switch (filename[i]) {
                    case ' ': // Space
                        s += '+';
                        break;

                    case '\\': // Back slash
                    case '/':  // Slash
                        s += '_';
                        break;

                    case '*': // Asterisk
                        s += '$';
                        break;

                    case '?': // Question mark
                        s += '=';
                        break;

                    case ':': // Colon
                    case '|': // Vertical line
                    case '>': // Greater than
                    case '<': // Small than
                        break;

                    default:
                        s += filename[i];
                }
            }

            return s;
        }

    }

    //
    // Port from QSynthesis end
    //

    struct CorrectGenon {
        double PreUtterance;
        double VoiceOverlap;
        double StartPoint;
    };

    // https://shinta0806be.ldblog.jp/archives/8298940.html
    static CorrectGenon getCorrectGenonSettings(double preUttr, double overlap, double stp,
                                                double velocity, double duration,
                                                double prevDuration, bool prevIsRest) {
        double correctRate = 1;
        double velocityRate = std::pow(2, 1 - velocity / 100);
        preUttr *= velocityRate;
        overlap *= velocityRate;
        if (prevDuration == 0) {
            return {preUttr, overlap, stp};
        }

        double maxOccupy = prevIsRest ? prevDuration : (prevDuration / 2);
        if (preUttr - overlap > maxOccupy) {
            correctRate = maxOccupy / (preUttr - overlap);
        }

        double correctPreUttr = correctRate * preUttr;
        double CorrectOverlap = correctRate * overlap;
        double CorrectSTPoint = preUttr - correctPreUttr + stp;

        // Voice Overlap shouldn't be too long
        if (CorrectOverlap - correctPreUttr > duration) {
            CorrectOverlap = correctPreUttr + duration;
        }

        return {correctPreUttr, CorrectOverlap, CorrectSTPoint};
    }

    static std::vector<double> getRawVibrato(const std::optional<Vibrato> &vbr) {
        if (!vbr) {
            return {};
        }
        return {
            vbr->length,  vbr->period, vbr->amplitude, vbr->attack,
            vbr->release, vbr->phase,  vbr->offset,    vbr->intensity,
        };
    }

    static std::vector<Point> getRawEnvelope(const std::optional<Envelope> &env) {
        if (!env) {
            return {};
        }

        std::vector<Point> res;
        for (int i = 0; i < 4; ++i) {
            res.push_back(env->anchors[i]);
        }
        if (env->count() > 4)
            res.push_back(env->anchors[4]);
        return res;
    }

    std::vector<std::string> ResamplerArguments::arguments() const {
        std::vector<std::string> list;

        list << inFile;   // Arg 1: Input file (Normally a sample in voicebank folder)
        list << outFile;  // Arg 2: Output file (Normally a cache file)
        list << toneName; // Arg 3: Tone Name

        list << std::to_string(velocity);   // Arg 4: Consonant Velocity
        list << flags;                      // Arg 5: Flags

        list << std::to_string(offset);     // Arg 6: Offset (Oto)
        list << std::to_string(realLength); // Arg 7: Corrected Duration
        list << std::to_string(consonant);  // Arg 8: Consonant (Oto)
        list << std::to_string(blank);      // Arg 9: Blank (Oto)

        list << params();

        return list;
    }

    std::vector<std::string> ResamplerArguments::params() const {
        std::vector<std::string> list;

        list << std::to_string(intensity);
        list << std::to_string(modulation);

        if (toBase64) {
            list << "!" + std::to_string(tempo);
            list << UtaPitchCurves::encode_from_vector(pitchCurves);
        } else {
            // No using Base 64
            if (pitchCurves.empty()) {
                list << std::string("0") + "Q" + std::to_string(tempo);
            } else {
                list << std::to_string(pitchCurves.front()) + "Q" + std::to_string(tempo);
                for (int i = 1; i < pitchCurves.size(); ++i) {
                    list << std::to_string(pitchCurves.at(i));
                }
            }
        }

        return list;
    }

    std::string WavtoolArguments::outDuration() const {
        std::string outDuration = std::to_string(length) + "@" + std::to_string(tempo);
        outDuration += ((correction >= 0) ? "+" : "") + std::to_string(correction);
        return outDuration;
    }

    std::vector<std::string> WavtoolArguments::env() const {
        std::vector<std::string> list;

        if (rest) {
            list << "0"
                 << "0";
        } else {
            list << UtaTranslator::EnvelopeToStringList(envelope, voiceOverlap);
        }

        return list;
    }

    std::vector<std::string> WavtoolArguments::arguments() const {
        std::vector<std::string> list;

        list << outFile; // Arg 1: Input File (Normally a cache file generated by resampler)
        list << inFile;  // Arg 2: Output File

        list << std::to_string(startPoint); // STP
        list << outDuration();              // Fixed Duration

        list << env();

        return list;
    }

    void Synth::calc(const std::pair<int, int> &rangeLimits, const std::pair<int, int> &range,
                     double initialTempo, const std::string &globalFlags,
                     const NoteGetter &noteGetter, const GenonSettingsGetter &genonSettingsGetter,
                     std::vector<std::pair<ResamplerArguments, WavtoolArguments>> *result) {

        int left = std::max(rangeLimits.first, range.first);
        int right = std::min(rangeLimits.second, range.second);
        if (range.first > range.second)
            return;

        // Get initial tempo
        double currentTempo = initialTempo;
        double prevDuration = 0;
        bool prevIsRest = false;
        if (left == 0) {
            const auto &note = noteGetter(0);
            if (note.hasTempo()) {
                currentTempo = note.tempo;
            }
        } else {
            for (int i = left - 1; i >= 0; --i) {
                const auto &note = noteGetter(i);
                if (!note.hasTempo()) {
                    continue;
                }
                currentTempo = note.tempo;
                break;
            }

            const auto &prevNote = noteGetter(left - 1);
            prevDuration = Note::duration(prevNote.length, currentTempo);
            prevIsRest = isRestLyric(prevNote.lyric);

            const auto &note = noteGetter(left);
            if (note.hasTempo()) {
                currentTempo = note.tempo;
            }
        }

        std::vector<std::pair<ResamplerArguments, WavtoolArguments>> args;
        for (int i = left; i <= right; ++i) {
            const auto &aNote = noteGetter(i);
            const auto &aPrevNote = noteGetter(i - 1);
            const auto &aNextNote = noteGetter(i + 1);

            int aNoteNum = aNote.noteNum;
            int aLength = aNote.length;

            const auto &aFlags = aNote.flags;
            const auto &aLyric = aNote.lyric;

            double aTempo;
            if (aNote.hasTempo()) {
                aTempo = aNote.tempo;
                currentTempo = aTempo;
            } else {
                aTempo = currentTempo;
            }

            double aIntensity = aNote.realIntensity();
            double aModulation = aNote.realModulation();
            double aVelocity = aNote.realVelocity();
            const auto &aGenon = genonSettingsGetter(aNote);

            double duration = Note::duration(aLength, aTempo);
            auto aCorrect = getCorrectGenonSettings(
                aNote.hasPreUtterance() ? aNote.preUttr : aGenon.preUtterance,
                aNote.hasVoiceOverlap() ? aNote.overlap : aGenon.voiceOverlap,
                aNote.realStartPoint(), aVelocity, duration, prevDuration, prevIsRest);
            prevDuration = duration;
            prevIsRest = isRestLyric(aLyric);

            double aPreUttr = aCorrect.PreUtterance;
            double aOverlap = aCorrect.VoiceOverlap;
            double aStartPoint = aCorrect.StartPoint;

            std::vector<Point> aPitch;
            std::vector<Point> aEnvelope;
            std::vector<double> aVibrato;

            // Compute Mode2 Pitch Bend
            std::vector<Point> aPrevPitch;
            std::vector<double> aPrevVibrato;
            std::string aPrevLyric;

            int aPrevLength = 480;
            int aPrevNoteNum = aNoteNum;

            // Previous Note
            if (i > rangeLimits.first) {
                auto aPrevNote = noteGetter(i - 1);
                aPrevLength = aPrevNote.length;
                aPrevLyric = aPrevNote.lyric;
                aPrevNoteNum = aPrevNote.noteNum;
                aPrevPitch = aPrevNote.portamento;               // Mode2 Pitch Control Points
                aPrevVibrato = getRawVibrato(aPrevNote.vibrato); // Mode2 Vibrato

                if (!aPrevPitch.empty() && i > rangeLimits.first + 1) {
                    const auto &noteBeforePrev = noteGetter(i - 2);
                    UtaTranslator::getCorrectPBSY(noteBeforePrev.noteNum, noteBeforePrev.lyric,
                                                  aPrevNoteNum, aPrevPitch.front());
                }
            }

            // Current Note
            aPitch = aNote.portamento;                  // Mode2 Mode2 Pitch Control Points
            aEnvelope = getRawEnvelope(aNote.envelope); // Envelope
            aVibrato = getRawVibrato(aNote.vibrato);    // Mode2 Vibrato

            // Correct the y coordinate of first point
            if (!aPitch.empty()) {
                UtaTranslator::getCorrectPBSY(aPrevNoteNum, aPrevLyric, aNoteNum, aPitch.front());
            }

            int aNextLength = 480;
            double aNextPreUttr = 0;
            double aNextOverlap = 0;
            std::vector<Point> aNextPitch;
            std::vector<double> aNextVibrato;

            // Next Note
            if (i < rangeLimits.second) {
                const auto &nextNote = noteGetter(i + 1);
                const auto &nextGenonSettings = genonSettingsGetter(nextNote);
                double nextTempo = nextNote.hasTempo() ? nextNote.tempo : currentTempo;
                auto nextGenon = getCorrectGenonSettings(
                    nextNote.hasPreUtterance() ? nextNote.preUttr : nextGenonSettings.preUtterance,
                    nextNote.hasVoiceOverlap() ? nextNote.overlap : nextGenonSettings.voiceOverlap,
                    nextNote.realStartPoint(), nextNote.realVelocity(),
                    Note::duration(aNextNote.length, nextTempo), prevDuration, prevIsRest);

                int aNextNoteNum = aNextNote.noteNum;            // Note Num
                aNextLength = aNextNote.length;                  // Length

                aNextPreUttr = nextGenon.PreUtterance;           // PreUtterance
                aNextOverlap = nextGenon.VoiceOverlap;           // Overlap

                aNextPitch = aNextNote.portamento;               // Mode2 Pitch Control Points
                aNextVibrato = getRawVibrato(aNextNote.vibrato); // Mode2 Vibrato

                // Correct the y coordinate of first point
                if (!aNextPitch.empty()) {
                    UtaTranslator::getCorrectPBSY(aNoteNum, aLyric, aNextNoteNum,
                                                  aNextPitch.front());
                }
            }

            if (aPitch.empty()) {
                aPitch = UtaTranslator::getDefaultPitch(aPrevNoteNum, aPrevLyric, aNoteNum);
            }

            // Convert Mode2 to Mode1
            std::vector<int> aPitchValues = UtaPitchCurves::convert_from_vector_point(
                aTempo, aPitch, aVibrato, aPreUttr, aStartPoint, aLength, aNextPitch, aNextVibrato,
                aNextPreUttr, aNextOverlap, aNextLength, aPrevPitch, aPrevVibrato, aPrevLength);

            // Real Length
            double aDuration = (double(aLength) / 480 * 60 / aTempo * 1000); // 由 ticks 换算长度
            double aDurationFix = aPreUttr - aNextPreUttr + aNextOverlap;

            double aRealLength = aDuration + aDurationFix + aStartPoint + 50;
            aRealLength = (aRealLength < aGenon.consonant) ? aGenon.consonant : aRealLength;
            aRealLength = int((aRealLength + 25) / 50) * 50;

            // Cache Name
            auto aToneName = toneNumToToneName(aNoteNum);
            auto cacheName = std::to_string(i) + "_" + UtaTranslator::fixFilename(aLyric) + "_" +
                             aToneName + "_" + std::to_string(aLength) + ".wav";

            // COnstruct arguments
            ResamplerArguments res;
            res.sequence = i;
            res.offset = aGenon.offset;
            res.consonant = aGenon.consonant;
            res.blank = aGenon.blank;
            res.toneName = aToneName;
            res.inFile = aGenon.fileName;
            res.outFile = cacheName;
            res.intensity = aIntensity;
            res.modulation = aModulation;
            res.velocity = aVelocity;
            res.flags = UtaTranslator::fixFlags(globalFlags + aFlags);
            res.tempo = aTempo;
            res.pitchCurves = aPitchValues;
            res.realLength = aRealLength;
            res.correctPreUttr = aCorrect.PreUtterance;
            res.correctOverlap = aCorrect.VoiceOverlap;
            res.correctStp = aCorrect.StartPoint;

            WavtoolArguments wav;
            wav.inFile = cacheName;
            wav.outFile = {};              // Later
            wav.startPoint = aStartPoint;
            wav.length = aLength;          // Out Duration Arg 1
            wav.tempo = aTempo;            // Out Duration Arg 2
            wav.correction = aDurationFix; // Out Duration Arg 3
            wav.voiceOverlap = aOverlap;
            wav.envelope = aEnvelope;

            if (prevIsRest) {
                wav.rest = true;
            }

            args.push_back({res, wav});
        }

        *result = args;
    }

}