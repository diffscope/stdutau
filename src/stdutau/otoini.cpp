#include "otoini.h"

#include <sstream>
#include <fstream>

#include "utautils.h"

namespace Utau {

    static OtoEntry parseEntry(const std::string_view &s) {
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
            tokenList.emplace_back("0"); // If the following entry is missing, we simply fill with 0
        }

        OtoEntry res;
        res.fileName = key;
        res.alias = tokenList[0];
        res.offset = stod2(tokenList[1]);
        res.consonant = stod2(tokenList[2]);
        res.cutoff = stod2(tokenList[3]);
        res.preUtterance = stod2(tokenList[4]);
        res.voiceOverlap = stod2(tokenList[5]);
        return res;
    }

    static std::string formatEntry(const OtoEntry &entry) {
        std::stringstream out;
        out << entry.fileName << EQUAL;
        out << entry.alias << COMMA;
        out << entry.offset << COMMA;
        out << entry.consonant << COMMA;
        out << entry.cutoff << COMMA;
        out << entry.preUtterance << COMMA;
        out << entry.voiceOverlap;
        return out.str();
    }

    OtoIni::OtoIni() = default;

    bool OtoIni::read(std::istream &is) {
        std::string line;
        while (readLine(is, line)) {
            if (line.empty()) {
                continue;
            }

            auto entry = parseEntry(line);
            const auto &fileName = entry.fileName;
            if (fileName.empty())
                continue;

            auto it = contents.find(fileName);
            if (it == contents.end()) {
                contents[fileName].push_back(entry);
            } else {
                it->second.push_back(entry);
            }
        }
        return true;
    }

    bool OtoIni::write(std::ostream &os) const {
        for (const auto &item : contents) {
            for (const auto &entry : item.second) {
                os << formatEntry(entry) << std::endl;
                if (!os.good())
                    return false;
            }
        }
        return true;
    }

}