#include "note.h"

#include <sstream>
#include <charconv>

#include "utautils.h"

namespace Utau {

    /*!
        \class Point
        \brief Utau point type.
    */

    /*!
        Converts string to point type.
    */
    Point::Type Point::stringToType(const std::string_view &s) {
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

    /*!
        Converts point type to string.
    */
    std::string Point::typeToString(Type type) {
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
        Returns a comma separated string as the representation of the vibrato.
    */
    std::string Vibrato::toString() const {
        std::stringstream ss;
        ss << length << COMMA;
        ss << period << COMMA;
        ss << amplitude << COMMA;
        ss << attack << COMMA;
        ss << release << COMMA;
        ss << phase << COMMA;
        ss << offset << COMMA;
        ss << intensity;
        return ss.str();
    }

    /*!
        Returns a vibrato parsed from the string.
    */
    Vibrato Vibrato::fromString(const std::string_view &s) {
        auto nums = split(s, {&COMMA, 1});
        if (nums.size() < 7)
            return {};

        Vibrato vbr;
        vbr.length = stod2(nums[0]);
        vbr.period = stod2(nums[1]);
        vbr.amplitude = stod2(nums[2]);
        vbr.attack = stod2(nums[3]);
        vbr.release = stod2(nums[4]);
        vbr.phase = stod2(nums[5]);
        vbr.offset = stod2(nums[6]);
        vbr.intensity = stod2(nums.size() >= 8 ? nums[7] : 0);
        return vbr;
    }

    /*!
        \class Envelope
        \brief Utau envelope structure type.
    */

    /*!
        Returns a comma separated string as the representation of the envelope.
    */
    std::string Envelope::toString() const {
        int offset;
        std::vector<double> nums;
        std::vector<std::string> res;

        offset = (count() == 5);
        nums.push_back(anchors.at(0).x);
        nums.push_back(anchors.at(1).x);
        nums.push_back(anchors.at(2 + offset).x);
        nums.push_back(anchors.at(0).y);
        nums.push_back(anchors.at(1).y);
        nums.push_back(anchors.at(2 + offset).y);
        nums.push_back(anchors.at(3 + offset).y);
        nums.push_back(anchors.at(3 + offset).x);

        if (count() == 5) {
            nums.push_back(anchors.at(2).x);
            nums.push_back(anchors.at(2).y);
        }
        if (nums.size() == 8 && nums.at(7) == 0.0) {
            nums.pop_back();
        }
        for (auto num : std::as_const(nums)) {
            res.push_back(to_string(num));
        }
        if (res.size() > 7) {
            res.insert(res.begin() + 7, "%");
        }
        return join(res, {&COMMA, 1});
    }

    /*!
        Returns an envelope parsed from the string.
    */
    Envelope Envelope::fromString(const std::string_view &s) {
        auto strList = split(s, {&COMMA, 1});
        std::vector<double> nums;

        if (strList.size() >= 8) {
            strList.erase(strList.begin() + 7); // Remove %
        } else if (strList.size() < 7) {
            return {};                          // Invalid
        }

        for (const auto &item : std::as_const(strList)) {
            nums.push_back(stod2(item));
        }
        if (nums.size() % 2 != 0) {
            nums.push_back(0);
        }

        Envelope env;
        int index = 0;
        env.anchors[index++] = {nums.at(0), nums.at(3)};
        env.anchors[index++] = {nums.at(1), nums.at(4)};
        if (nums.size() == 10) {
            env.anchors[index++] = {nums.at(8), nums.at(9)};
        }
        env.anchors[index++] = {nums.at(2), nums.at(5)};
        env.anchors[index++] = {nums.at(7), nums.at(6)};
        return env;
    }

    /*!
        \class Note
        \brief Utau note structure type.
    */

    /*!
        Constructs a note from the given key, length and lyric.
    */
    Note::Note(int noteNum, int length, const std::string &lyric)
        : noteNum(noteNum), length(length), lyric(lyric) {
        velocity = preUttr = overlap = stp = Utau::NODEF_DOUBLE;

        intensity = 100.0;
        modulation = 0.0;

        tempo = NODEF_DOUBLE;
        pbstart = NODEF_DOUBLE;

        pbtype = VALUE_PITCH_TYPE;
    }

    /*!
        Converts the pitch bend strings to a point vector.
    */
    std::vector<Point> PBStrings::toPoints() const {
        if (PBS.empty() || PBW.empty()) {
            return {};
        }

        Point p;
        auto PBSXY = split(PBS, ";");
        if (!PBSXY.empty()) {
            p.x = stod2(PBSXY.front(), p.x);
            if (PBSXY.size() >= 2) {
                p.y = stod2(PBSXY.at(1), p.y);
            }
        }

        std::vector<Point> res;
        res.push_back(p);

        auto PBWs = split(PBW, ",");
        auto PBYs = split(PBY, ",");
        auto PBMs = split(PBM, ",");

        for (int i = 0; i < std::max(PBWs.size(), PBYs.size()); i++) {
            p = {};
            if (PBWs.size() > i) {
                p.x = stod2(PBWs.at(i), p.x);
            }
            if (PBYs.size() > i) {
                if (!PBYs.at(i).empty())
                    p.y = stod2(PBYs.at(i), p.y);
            }
            if (PBMs.size() > i) {
                p.type = Point::stringToType(PBMs.at(i));
            }
            p.x += res.back().x;
            res.push_back(p);
        }

        // Fix Negative Correction
        for (int i = 1; i < res.size(); ++i) {
            auto &curPoint = res[i];
            auto &prevPoint = res[i - 1];

            if (curPoint.x < prevPoint.x) {
                curPoint.x = prevPoint.x;
            }
        }

        return res;
    }

    /*!
        Converts the point vector to pitch bend strings.
    */
    PBStrings PBStrings::fromPoints(const std::vector<Point> &points) {
        PBStrings res;
        if (points.empty()) {
            return res;
        }

        // PBS
        {
            const Point &first = points.front();
            res.PBS = to_string(first.x) + ";" + to_string(first.y);
        }

        // PBW
        {
            std::vector<std::string> strs;
            for (int i = 1; i < points.size(); i++) {
                strs.push_back(to_string(points.at(i).x - points.at(i - 1).x));
            }
            res.PBW = join(strs, ",");
        }

        // PBY
        {
            std::vector<std::string> strs;
            strs.clear();
            for (int i = 1; i < points.size(); i++) {
                strs.push_back(to_string(points.at(i).y));
            }
            res.PBY = join(strs, ",");
        }

        // PBM
        {
            std::vector<std::string> strs;
            strs.clear();
            for (int i = 1; i < points.size(); i++) {
                strs.push_back(Point::typeToString(points.at(i).type));
            }
            res.PBM = join(strs, ",");
        }

        return res;
    }

}