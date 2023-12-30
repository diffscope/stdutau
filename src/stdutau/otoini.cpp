#include "otoini.h"

#include <sstream>
#include <fstream>

#include "utautils.h"

namespace Utau {

    static GenonSettings StringToGenon(const std::string_view &s) {
        if (s.empty()) {
            return {};
        }

        auto eq = s.find(EQUAL);
        if (eq == std::string::npos) {
            return {};
        }

        auto key = s.substr(0, eq);
        auto tokens = s.substr(eq + 1);
        auto tokenList = split(tokens, {&COMMA, 1});
        if (tokenList.empty())
            return {}; // No file name is not acceptable

        while (tokenList.size() < 6) {
            tokenList.push_back("0"); // If the following entry is missing, we simply fill with 0
        }

        GenonSettings res;
        res.fileName = key;
        res.alias = tokenList[0];
        res.offset = stod2(tokenList[1]);
        res.cosonant = stod2(tokenList[2]);
        res.blank = stod2(tokenList[3]);
        res.preUtterance = stod2(tokenList[4]);
        res.voiceOverlap = stod2(tokenList[5]);
        return res;
    }

    static std::string GenonToString(const GenonSettings &genon) {
        std::stringstream out;
        out << genon.fileName << EQUAL;
        out << genon.alias << COMMA;
        out << genon.offset << COMMA;
        out << genon.cosonant << COMMA;
        out << genon.blank << COMMA;
        out << genon.preUtterance << COMMA;
        out << genon.voiceOverlap;
        return out.str();
    }

    /*!
        \class OtoIni
        \brief UTAU original tone profile(oto.ini) reader and writer.
    */

    /*!
        Constructor.
    */
    OtoIni::OtoIni() = default;

    /*!
        Reads \c oto.ini items from file, returns \c true if succees.
    */
    bool OtoIni::load(const std::filesystem::path &path) {
        std::ifstream fs(path);
        if (!fs.is_open())
            return false;

        std::string line;
        while (std::getline(fs, line)) {
            if (line.empty()) {
                continue;
            }

            auto genon = StringToGenon(line);
            const auto &fileName = genon.fileName;
            if (fileName.empty())
                continue;

            auto it = contents.find(fileName);
            if (it == contents.end()) {
                contents[fileName].push_back(genon);
            } else {
                it->second.push_back(genon);
            }
        }
        return true;
    }

    /*!
        Writes \c oto.ini items to file, returns \c true if succees.
    */
    bool OtoIni::save(const std::filesystem::path &path) const {
        std::ofstream fs(path);
        if (!fs.is_open())
            return false;

        for (const auto &item : contents) {
            for (const auto &genon : item.second) {
                fs << GenonToString(genon) << std::endl;
            }
        }
        return true;
    }

}