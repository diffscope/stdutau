#include "note.h"

#include "utaconst.h"

namespace Utau {

    /*!
        \class Point
        \brief Utau point type.
    */

    Point::Type Point::StringToType(const std::string &s) {
        Type res = sJoin;
        if (s == "s") {
            res = linearJoin;
        } else if (s == "r") {
            res = rJoin;
        } else if (s == "j") {
            res = jJoin;
        }
        return res;
    }

    std::string Point::TypeToString(Type type) {
        std::string res;
        switch (type) {
            case linearJoin:
                res = "s";
                break;
            case rJoin:
                res = "r";
                break;
            case jJoin:
                res = "j";
                break;
            default:
                break;
        }
        return res;
    }

    /*!
        \class Vibrato
        \brief Utau vibrato structure type.
    */

    /*!
        \class Envelope
        \brief Utau envelope structure type.
    */

    /*!
        \class Note
        \brief Utau note structure type.
    */

    Note::Note(int noteNum, int length, const std::string &lyric)
        : noteNum(noteNum), length(length), lyric(lyric) {
        velocity = preUttr = overlap = stp = Utau::NODEF_DOUBLE;
        intensity = NODEF_INT;
        modulation = 0.0;
        tempo = NODEF_DOUBLE;
        pbstart = NODEF_DOUBLE;
    }

}