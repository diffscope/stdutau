#ifndef NOTE_H
#define NOTE_H

#include <array>
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

        static Type StringToType(const std::string &s);
        static std::string TypeToString(Type type);

    public:
        double x;
        double y;
        Type type;
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
        static Vibrato fromString(const std::string &s);

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
        : length(65), period(180), amplitude(35), attack(200), release(20), phase(0), offset(0),
          intensity(0) {
    }

    class STDUTAU_EXPORT Envelope {
    public:
        inline constexpr Envelope();

        inline constexpr int count() const;

        std::string toString() const;
        static Envelope fromString(const std::string &s);

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
        Note(int noteNum, int length, const std::string &lyric = "a");

        inline constexpr double realIntensity() const;
        inline constexpr double realModulation() const;
        inline constexpr double realVelocity() const;
        inline constexpr double realPreUtterance() const;
        inline constexpr double realVoiceOverlap() const;
        inline constexpr double realStartPoint() const;
        inline constexpr double hasTempo() const;
        inline constexpr double hasPBStart() const;

    public:
        // String
        std::string lyric, flags;

        // Position
        int noteNum;
        int length;

        // Doubles
        double intensity, modulation, velocity;
        double preUttr, overlap, stp;
        double tempo;

        // Envelope
        std::optional<Envelope> envelope;

        // Mode 2
        std::vector<Point> portamento;
        std::optional<Vibrato> vibrato;

        // Mode 1
        double pbstart;
        std::vector<double> pitches;
        std::string pbtype;

        // Labels
        std::string label;
        std::string direct;
        std::string patch;
        std::string region;
        std::string regionEnd;

        // User
        std::vector<std::pair<std::string, std::string>> userData;
    };

    inline Note::Note() : Note(60, 480) {
    }

    inline constexpr double Note::realIntensity() const {
        return intensity == NODEF_DOUBLE ? DEFAULT_VALUE_INTENSITY : intensity;
    }

    inline constexpr double Note::realModulation() const {
        return intensity == NODEF_DOUBLE ? DEFAULT_VALUE_MODULATION : intensity;
    }

    inline constexpr double Note::realVelocity() const {
        return intensity == NODEF_DOUBLE ? DEFAULT_VALUE_VELOCITY : intensity;
    }

    inline constexpr double Note::realPreUtterance() const {
        return intensity == NODEF_DOUBLE ? DEFAULT_VALUE_PRE_UTTERANCE : intensity;
    }

    inline constexpr double Note::realVoiceOverlap() const {
        return intensity == NODEF_DOUBLE ? DEFAULT_VALUE_VOICE_OVERLAP : intensity;
    }

    inline constexpr double Note::realStartPoint() const {
        return intensity == NODEF_DOUBLE ? DEFAULT_VALUE_START_POINT : intensity;
    }

    inline constexpr double Note::hasTempo() const {
        return tempo != NODEF_DOUBLE;
    }

    inline constexpr double Note::hasPBStart() const {
        return pbstart != NODEF_DOUBLE;
    }

    struct PBStrings {
        std::string PBS;
        std::string PBW;
        std::string PBY;
        std::string PBM;

        std::vector<Point> toPoints() const;
        static PBStrings fromPoints(const std::vector<Point> &points);
    };

}

#endif // NOTE_H
