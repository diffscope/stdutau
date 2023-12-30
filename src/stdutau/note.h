#ifndef NOTE_H
#define NOTE_H

#include <array>
#include <string>
#include <vector>

#include <stdutau/utaglobal.h>

namespace Utau {

    class STDUTAU_EXPORT Point {
    public:
        enum Type {
            sJoin,
            linearJoin,
            rJoin,
            jJoin,
        };

        inline Point();
        inline Point(double x, double y);
        inline Point(double x, double y, Type p);

        inline bool operator==(const Point &other) const;
        inline bool operator!=(const Point &other) const;
        inline bool operator<(const Point &other) const;

        static Type StringToType(const std::string &s);
        static std::string TypeToString(Type type);

    public:
        double x;
        double y;
        Type type;
    };

    inline Point::Point() : Point(0.0, 0.0, sJoin) {
    }

    inline Point::Point(double x, double y) : Point(x, y, sJoin) {
    }

    inline Point::Point(double x, double y, Type t) : x(x), y(y), type(t) {
    }

    inline bool Point::operator==(const Point &other) const {
        return (other.x == x && other.y == y && other.type == type);
    }

    inline bool Point::operator!=(const Point &other) const {
        return !((*this) == other);
    }

    inline bool Point::operator<(const Point &other) const {
        return x < other.x;
    }

    class STDUTAU_EXPORT Vibrato {
    public:
        inline Vibrato();

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

    inline Vibrato::Vibrato()
        : length(65), period(180), amplitude(35), attack(200), release(20), phase(0), offset(0),
          intensity(0) {
    }

    class Envelope {
    public:
        inline Envelope();

        std::array<Point, 5> anchors;
    };

    inline Envelope::Envelope()
        : anchors({Point(0, 0), Point(5, 100), Point(35, 100), Point(0, 0), Point(-1, -1)}) {
    }


    class STDUTAU_EXPORT Note {
    public:
        inline Note();
        Note(int noteNum, int length, const std::string &lyric = "a");

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
        Envelope envelope;

        // Mode 2
        std::vector<Point> Mode2Pitch;
        Vibrato vibrato;

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

}

#endif // NOTE_H
