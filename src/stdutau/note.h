#ifndef NOTE_H
#define NOTE_H

#include <array>
#include <map>
#include <string>
#include <vector>
#include <optional>

#include <stdutau/utaglobal.h>
#include <stdutau/utaconst.h>

namespace Utau {

    class STDUTAU_EXPORT Point {
    public:
        enum Type {
            sJoin,
            linearJoin,
            rJoin,
            jJoin,
        };

        inline constexpr Point();
        inline constexpr Point(double x, double y);
        inline constexpr Point(double x, double y, Type p);

        inline constexpr bool operator==(const Point &other) const;
        inline constexpr bool operator!=(const Point &other) const;
        inline constexpr bool operator<(const Point &other) const;

        static Type stringToType(const std::string_view &s);
        static std::string typeToString(Type type);

    public:
        double x;
        double y;
        Type type; // Not used in envelope
    };

    inline constexpr Point::Point() : Point(0.0, 0.0, sJoin) {
    }

    inline constexpr Point::Point(double x, double y) : Point(x, y, sJoin) {
    }

    inline constexpr Point::Point(double x, double y, Type t) : x(x), y(y), type(t) {
    }

    inline constexpr bool Point::operator==(const Point &other) const {
        return (other.x == x && other.y == y && other.type == type);
    }

    inline constexpr bool Point::operator!=(const Point &other) const {
        return !((*this) == other);
    }

    inline constexpr bool Point::operator<(const Point &other) const {
        return x < other.x;
    }

    class STDUTAU_EXPORT Vibrato {
    public:
        inline constexpr Vibrato();

        std::string toString() const;
        static Vibrato fromString(const std::string_view &s);

    public:
        double length;
        double period;
        double amplitude;
        double attack;
        double release;
        double phase;
        double offset;
        double intensity; // Not used
    };

    inline constexpr Vibrato::Vibrato()
        : length(65), period(180), amplitude(35), attack(20), release(20), phase(0), offset(0),
          intensity(0) {
    }

    class STDUTAU_EXPORT Envelope {
    public:
        inline constexpr Envelope();

        inline constexpr int count() const;

        std::string toString() const;
        static Envelope fromString(const std::string_view &s);

    public:
        std::array<Point, 5> anchors;
    };

    inline constexpr Envelope::Envelope()
        : anchors({Point(0, 0), Point(5, 100), Point(35, 100), Point(0, 0), Point(-1, -1)}) {
    }

    inline constexpr int Envelope::count() const {
        return anchors[4].y >= 0 ? 5 : 4;
    }

    class STDUTAU_EXPORT Note {
    public:
        inline Note();
        Note(int noteNum, int length, const std::string &lyric = DEFAULT_LYRIC);
        virtual ~Note() = default;

        inline constexpr double realIntensity() const;
        inline constexpr double realModulation() const;
        inline constexpr double realVelocity() const;
        inline constexpr double realStartPoint() const;

        static inline constexpr double duration(int length, double tempo);

    public:
        std::string lyric, flags;

        int noteNum;
        int length;

        /// Absent where the file leaves the entry out, which is how UTAU says to use the value
        /// from the project or the voice bank instead.
        std::optional<double> intensity, modulation, velocity;
        std::optional<double> preUttr, overlap, stp;
        std::optional<double> tempo;

        std::optional<Envelope> envelope;

        std::vector<Point> portamento;
        std::optional<Vibrato> vibrato;

        std::optional<double> pbstart;
        std::vector<double> pitches;
        std::string pbtype;

        std::string label;
        std::string direct, patch;
        std::string region, regionEnd;

        /// Entries found on the note that this class does not otherwise represent. They are kept
        /// as they were read and written back at the end of the section, so a host can store its
        /// own data on a note and find it again.
        ///
        /// The map is keyed by the entry name, which makes a repeated name resolve to the last
        /// one read. UTAU resolves it the same way.
        ///
        /// \note UTAU keeps an entry it does not know only when the name begins with \c $ , and
        ///       only on a note section. Anything else stored here survives a round trip through
        ///       this library and is gone the moment the file passes through UTAU. The value is
        ///       subject to further limits there: \c = and a tab truncate it, a space becomes a
        ///       comma, and an empty value drops the entry.
        std::map<std::string, std::string> userData;
    };

    inline Note::Note() : Note(60, 480) {
    }

    inline constexpr double Note::realIntensity() const {
        return intensity.value_or(DEFAULT_VALUE_INTENSITY);
    }

    inline constexpr double Note::realModulation() const {
        return modulation.value_or(DEFAULT_VALUE_MODULATION);
    }

    inline constexpr double Note::realVelocity() const {
        return velocity.value_or(DEFAULT_VALUE_VELOCITY);
    }

    inline constexpr double Note::realStartPoint() const {
        return stp.value_or(DEFAULT_VALUE_START_POINT);
    }

    inline constexpr double Note::duration(int length, double tempo) {
        return length * 125.0 / tempo;
    }

    class NoteExt : public Note {
    public:
        inline NoteExt();
        inline NoteExt(int noteNum, int length, const std::string &lyric = DEFAULT_LYRIC);

    public:
        /// What UTAU worked out for the note, absent where the file did not say.
        std::optional<double> preUttrRO;
        std::optional<double> overlapRO;
        std::optional<double> stpRO;

        /// Empty where the file did not say. An empty alias is what a sample without one has, so
        /// there is nothing here for an optional to tell apart.
        std::string filenameRO;
        std::string aliasRO;
        std::string cacheRO;
    };

    inline NoteExt::NoteExt() {
    }

    inline NoteExt::NoteExt(int noteNum, int length, const std::string &lyric)
        : Note(noteNum, length, lyric) {
    }

    struct STDUTAU_EXPORT PBStrings {
        std::string PBS;
        std::string PBW;
        std::string PBY;
        std::string PBM;

        std::vector<Point> toPoints() const;
        static PBStrings fromPoints(const std::vector<Point> &points);
    };

}

#endif // NOTE_H
