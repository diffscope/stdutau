#include "synth.h"

#include <cmath>
#include <cstdint>

#include "utautils.h"

namespace utau {

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

        static constexpr const double PI = 3.1415926;

        static constexpr const char Base64EncodeMap[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

        static inline double f_x(double x1, double y1, double x2, double y2, double x) {
            if (x1 == x2) {
                return y1;
            }
            return (y1 - y2) / 2 * std::cos(PI * (x - x1) / (x2 - x1)) + (y1 + y2) / 2;
        }

        static inline double f_r(double x1, double y1, double x2, double y2, double x) {
            if (x1 == x2) {
                return y1;
            }
            return (y2 - y1) * std::cos(PI / 2 / (x2 - x1) * (x - x2)) + y1;
        }

        static inline double f_j(double x1, double y1, double x2, double y2, double x) {
            if (x1 == x2) {
                return y1;
            }
            return (y1 - y2) * std::cos(PI / 2 / (x2 - x1) * (x - x1)) + y2;
        }

        static inline constexpr double f_s(double x1, double y1, double x2, double y2, double x) {
            if (x1 == x2) {
                return y1;
            }
            return (y2 - y1) / (x2 - x1) * (x - x1) + y1;
        }

        static inline double f_type(const Point::Type &ptype, double x1, double y1, double x2,
                                    double y2, double x) {
            int impact;
            switch (ptype) {
                case Point::LinearJoin:
                    impact = int(f_s(x1, y1, x2, y2, x));
                    break;
                case Point::JJoin:
                    impact = int(f_j(x1, y1, x2, y2, x));
                    break;
                case Point::RJoin:
                    impact = int(f_r(x1, y1, x2, y2, x));
                    break;
                default:
                    impact = int(f_x(x1, y1, x2, y2, x));
                    break;
            }
            return impact;
        }

        /// The shortest vibrato UTAU draws, in milliseconds.
        ///
        /// Below this it draws nothing at all: not a small vibrato, none. Measured on a probe
        /// that sweeps the vibrato's length two percent at a time, where fifty milliseconds is
        /// dropped and sixty is drawn.
        ///
        /// It is the milliseconds that decide, not the ticks and not the cycles. The same
        /// ninety-six ticks of vibrato are drawn at 120 bpm and dropped at 240, and a fifty
        /// millisecond window holding two and a half whole cycles is dropped just the same.
        static constexpr const double SHORTEST_VIBRATO = 50;

        /// \brief Whether UTAU draws this vibrato at all
        /// \param length the note's length in ticks
        /// \param tempo the note's tempo, which is what turns its ticks into milliseconds
        static bool vibrato_is_drawn(const std::vector<double> &vibrato, int length,
                                     double tempo) {
            if (vibrato.size() < 8 || length <= 0 || tempo <= 0) {
                return false;
            }
            const double ticks = vibrato[0] / 100.0 * length;
            return ticks * 60000 / (tempo * 480) > SHORTEST_VIBRATO;
        }

        static double find_impact(const std::vector<Point> &portamento, int &startIndex,
                                  double curTick, double PositiveTempo, double NegativeTempo,
                                  const std::vector<double> &vibrato, int length) {

            // portamento: Mode2 Pitch curve points
            // startIndex: search from index
            // curTick: current tick, and a fraction of one is not thrown away. Reading the
            //          curve on whole ticks costs up to fifty cents on a fast vibrato, and the
            //          455-note probe says UTAU keeps the fraction.
            // PositiveTempo: tempo used when ticks is positive
            // NegativeTempo: tempo used when ticks is negative (necessary when tempo changed)
            // vibrato: The vibrato sequence, which can be absent

            if (startIndex < 0) {
                return 0;
            }

            double basePitch;
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
                    basePitch = portamento[0].y * 10; // haven't reached the first point yet
                }
            }

            // Search vibrato
            if (vibrato_is_drawn(vibrato, length, PositiveTempo)) {
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

                k = 1 / tick_time * 2 * PI; // Circular frequency
                p = phase / 100.0 * 2 * PI; // Initial phase

                easeIn = easeIn / 100.0 * tick_length;         // Fade in time
                easeOut = (1 - easeOut / 100.0) * tick_length; // Fade out time
                offset /= 100.0;

                x = curTick - tick_start;       // tick x
                y = amplitude * sin(k * x - p); // original vibrato

                if (x > 0 && x < tick_length) {
                    ratio = 1;
                    // Add offset
                    y += offset * amplitude;
                    // Calculate envelope. One or the other, never both: a fade in of 80% and a
                    // fade out of 80% overlap, and UTAU lets the fade in win the whole way. Its
                    // envelope climbs to 1.006 four fifths of the way through and drops to 0.245
                    // on the very next reading, which is the fade out picking up where it always
                    // would have. Multiplying the two instead flattens the middle to 0.39 and is
                    // out by sixty cents.
                    if (x < easeIn) {
                        ratio = x / easeIn;
                    } else if (x > easeOut) {
                        ratio = 1 - (x - easeOut) / (tick_length - easeOut);
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
            double tempo1, const std::vector<Point> &curNote, const std::vector<double> &curVBR,
            double curPre, double curStp, int curLength, const std::vector<Point> &nextNote,
            const std::vector<double> &nextVBR, double nextPre, double nextOve, int nextLength,
            const std::vector<Point> &prevNote, const std::vector<double> &prevVBR,
            int prevLength) {

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
            double basePitch, prevImpact, nextImpact;
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

            // One reading past the end. The probe's vibrato notes carry fifty readings where this
            // loop alone produces forty-nine, and the missing one is the last: without it the
            // curve stops five ticks early and an engine holding the last value holds the wrong
            // one, which on a fast vibrato is half a semitone.
            while (tick < duration + 5) {
                prevImpact = 0;
                nextImpact = 0;

                basePitch = find_impact(curNote, i, tick, curTempo, prevTempo, curVBR, curLength);

                // The part influenced by the next note
                if (tick >= nextStart) {
                    if (j < nextNote.size() - 1) {
                        nextImpact = find_impact(nextNote, j, tick - curLength, curTempo, curTempo,
                                                 nextVBR, nextLength);
                    }
                    nextImpact += -(nextNote[0].y * 10);
                }

                // The part influenced by the previous note
                if (tick <= 0) {
                    prevImpact = find_impact(prevNote, k, tick + prevLength, prevTempo, prevTempo,
                                             prevVBR, prevLength);
                }

                // Add the influence of the pitch line before and after the note. Rounded, not
                // truncated: on the probe's vibrato notes truncating misses UTAU by a cent on
                // nearly every reading, and rounding lands on its number.
                PitchBend.push_back(int(std::floor(basePitch + prevImpact + nextImpact + 0.5)));
                tick = tick + 5;
            }

            // The trailing zeros stay. A loop here used to drop them, which reads like a
            // saving and is not one: it only ever removes zeros, so what is left ends on
            // the last value that was not zero, and an engine that holds the last reading
            // then holds the note bent instead of letting it come back. UTAU sends them.

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
                            result += "#" + to_string(count) + "#";
                        } else {
                            result += encode_single_num(prevInt);
                        }
                    }
                } else {
                    if (count != 0) {
                        // Use n-1 to replace the rest when appear repeatedly
                        if (count >= 2) {
                            result += "#" + to_string(count) + "#";
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
                                                             double overlap) {
            std::string strOverlap = to_string(overlap);
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

            listEnv << to_string(tpoints[0].x);
            listEnv << to_string(tpoints[1].x);
            listEnv << to_string(tpoints[tpoints.size() - 2].x);
            listEnv << to_string(tpoints[0].y);
            listEnv << to_string(tpoints[1].y);
            listEnv << to_string(tpoints[tpoints.size() - 2].y);
            listEnv << to_string(tpoints[tpoints.size() - 1].y);
            listEnv << strOverlap;
            if (tpoints.size() == 5) {
                listEnv << to_string(tpoints[tpoints.size() - 1].x);
                listEnv << to_string(tpoints[2].x);
                listEnv << to_string(tpoints[2].y);
            } else if (tpoints[tpoints.size() - 1].x != 0) {
                listEnv << to_string(tpoints[tpoints.size() - 1].x);
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

        static std::vector<Point> getDefaultPitch(int prevNoteNum, const std::string &prevLyric,
                                                  int curNoteNum) {

            Point first(0, 0);
            Point second(0, 0);

            getCorrectPBSY(prevNoteNum, prevLyric, curNoteNum, first);
            second.type = Point::SJoin;

            std::vector<Point> pitch = {first, second};

            return pitch;
        }

        static std::string fixFlags(const std::string &s) {
            std::string s2;

            for (char ch : s) {
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

            for (char ch : filename) {
                switch (ch) {
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
                    case '<': // Smaller than
                        break;

                    default:
                        s += ch;
                }
            }

            return s;
        }

    }

    //
    // Port from QSynthesis end
    //

    struct CorrectedTiming {
        double PreUtterance;
        double VoiceOverlap;
        double StartPoint;
    };

    // https://shinta0806be.ldblog.jp/archives/8298940.html
    static CorrectedTiming correctedTiming(double preUttr, double overlap, double stp,
                                           double velocity, double duration, double prevDuration,
                                           bool prevIsRest) {
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

    static std::vector<Point> getRawEnvelope(const std::optional<Envelope> &envelope) {
        if (!envelope) {
            return {};
        }

        std::vector<Point> res;
        res.reserve(5);
        for (int i = 0; i < 4; ++i) {
            res.push_back(envelope->anchors[i]);
        }
        if (envelope->count() > 4)
            res.push_back(envelope->anchors[4]);
        return res;
    }

    std::vector<std::string> ResamplerArguments::trailingArguments() const {
        std::vector<std::string> list;

        list << to_string(intensity);
        list << to_string(modulation);

        if (toBase64) {
            list << "!" + to_string(tempo);
            list << UtaPitchCurves::encode_from_vector(pitchCurves);
        } else {
            // No using Base 64
            if (pitchCurves.empty()) {
                list << std::string("0") + "Q" + to_string(tempo);
            } else {
                list << to_string(pitchCurves.front()) + "Q" + to_string(tempo);
                for (int i = 1; i < pitchCurves.size(); ++i) {
                    list << to_string(pitchCurves[i]);
                }
            }
        }

        return list;
    }

    std::vector<std::string> ResamplerArguments::arguments() const {
        std::vector<std::string> list;

        list << inFile;   // Arg 1: Input file (Normally a sample in voicebank folder)
        list << outFile;  // Arg 2: Output file (Normally a cache file)
        list << toneName; // Arg 3: Tone Name

        list << to_string(velocity); // Arg 4: Consonant Velocity
        list << flags;               // Arg 5: Flags

        list << to_string(offset);     // Arg 6: Offset (Oto)
        list << to_string(realLength); // Arg 7: Corrected Duration
        list << to_string(consonant);  // Arg 8: Consonant (Oto)
        list << to_string(blank);      // Arg 9: Blank (Oto)

        list << trailingArguments();

        return list;
    }

    std::string WavtoolArguments::outDuration() const {
        std::string outDuration = to_string(length) + "@" + to_string(tempo);
        outDuration += ((correction >= 0) ? "+" : "") + to_string(correction);
        return outDuration;
    }

    std::vector<std::string> WavtoolArguments::envelopeArguments() const {
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

        list << outFile; // Arg 1: the track being built up, which this appends to
        list << inFile;  // Arg 2: the cache file the resampler produced

        list << to_string(startPoint); // STP
        list << outDuration();         // Fixed Duration

        list << envelopeArguments();

        return list;
    }

    Synth::SynthParams Synth::calc(const std::pair<int, int> &rangeLimits,
                                   const std::pair<int, int> &range, double initialTempo,
                                   const std::string &globalFlags, const NoteGetter &noteGetter,
                                   const OtoEntryGetter &otoEntryGetter) {

        int left = std::max(rangeLimits.first, range.first);
        int right = std::min(rangeLimits.second, range.second);
        if (range.first > range.second)
            return {};

        // Get initial tempo
        double currentTempo = initialTempo;
        double prevDuration = 0;
        bool prevIsRest = false;
        if (left == 0) {
            const auto &note = noteGetter(0);
            currentTempo = note.tempo.value_or(currentTempo);
        } else {
            for (int i = left - 1; i >= 0; --i) {
                const auto &note = noteGetter(i);
                if (!note.tempo) {
                    continue;
                }
                currentTempo = *note.tempo;
                break;
            }

            const auto &prevNote = noteGetter(left - 1);
            prevDuration = Note::duration(prevNote.length, currentTempo);
            prevIsRest = isRestLyric(prevNote.lyric);

            const auto &note = noteGetter(left);
            currentTempo = note.tempo.value_or(currentTempo);
        }

        SynthParams args;
        for (int i = left; i <= right; ++i) {
            const auto &aNote = noteGetter(i);
            const auto &aNextNote = noteGetter(i + 1);

            int aNoteNum = aNote.noteNum;
            int aLength = aNote.length;

            const auto &aFlags = aNote.flags;
            const auto &aLyric = aNote.lyric;

            double aTempo = aNote.tempo.value_or(currentTempo);
            currentTempo = aTempo;

            double aIntensity = aNote.realIntensity();
            double aModulation = aNote.realModulation();
            double aVelocity = aNote.realVelocity();
            const auto &aOto = otoEntryGetter(aNote);

            double duration = Note::duration(aLength, aTempo);
            auto aCorrect =
                correctedTiming(aNote.preUttr.value_or(aOto.preUtterance),
                                aNote.overlap.value_or(aOto.voiceOverlap), aNote.realStartPoint(),
                                aVelocity, duration, prevDuration, prevIsRest);
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
                const auto &nextOto = otoEntryGetter(nextNote);
                double nextTempo = nextNote.tempo.value_or(currentTempo);
                auto nextTiming = correctedTiming(
                    nextNote.preUttr.value_or(nextOto.preUtterance),
                    nextNote.overlap.value_or(nextOto.voiceOverlap), nextNote.realStartPoint(),
                    nextNote.realVelocity(), Note::duration(aNextNote.length, nextTempo),
                    prevDuration, prevIsRest);

                int aNextNoteNum = aNextNote.noteNum; // Note Num
                aNextLength = aNextNote.length;       // Length

                aNextPreUttr = nextTiming.PreUtterance; // PreUtterance
                aNextOverlap = nextTiming.VoiceOverlap; // Overlap

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

            // Not lifted to the consonant first. A line doing that was here, and it is wrong:
            // rendering a project under UTAU and reading the calls back out of the temp.bat it
            // wrote gives 96 of 96 notes matching without it and 87 of 96 with it. The nine that
            // tell the two apart are short notes whose sample has a long consonant, and UTAU
            // renders those shorter than the consonant rather than stretching to reach it.
            double aRealLength = aDuration + aDurationFix + aStartPoint + 50;
            aRealLength = int((aRealLength + 25) / 50) * 50;

            // Cache Name
            auto aToneName = toneNumToToneName(aNoteNum);
            auto cacheName = to_string(i) + "_" + UtaTranslator::fixFilename(aLyric) + "_" +
                             aToneName + "_" + to_string(aLength) + ".wav";

            // COnstruct arguments
            ResamplerArguments res;
            res.sequence = i;
            res.offset = aOto.offset;
            res.consonant = aOto.consonant;
            res.blank = aOto.cutoff;
            res.toneName = aToneName;
            res.inFile = aOto.fileName;
            res.outFile = cacheName;
            res.intensity = aIntensity;
            res.modulation = aModulation;
            res.velocity = aVelocity;
            // The note's own first, the project's after. Rendering a project under UTAU
            // and reading the calls out of its temp.bat gives g5B0 for a note carrying g5
            // under a project carrying B0, on 141 of the 164 notes that were asked; the
            // rest carry an e or an E and start with the slash that puts in front of one.
            res.flags = UtaTranslator::fixFlags(aFlags + globalFlags);
            res.tempo = aTempo;
            res.pitchCurves = aPitchValues;
            res.realLength = aRealLength;
            res.correctPreUttr = aCorrect.PreUtterance;
            res.correctOverlap = aCorrect.VoiceOverlap;
            res.correctStp = aCorrect.StartPoint;

            WavtoolArguments wav;
            wav.inFile = cacheName;
            wav.outFile = {}; // Later
            wav.startPoint = aStartPoint;
            wav.length = aLength;          // Out Duration Arg 1
            wav.tempo = aTempo;            // Out Duration Arg 2
            wav.correction = aDurationFix; // Out Duration Arg 3
            wav.voiceOverlap = aOverlap;
            wav.envelope = aEnvelope;

            if (prevIsRest) {
                wav.rest = true;
            }

            args.emplace_back(res, wav);
        }

        return args;
    }

}