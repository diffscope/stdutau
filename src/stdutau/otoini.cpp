#include "otoini.h"

#include <charconv>
#include <fstream>

#include "utautils.h"

namespace utau {

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
            tokenList.emplace_back("0"); // Missing trailing fields are filled with 0
        }

        OtoEntry res;
        res.fileName = key;
        res.alias = tokenList[0];
        res.offset = stod2(tokenList[1]);
        res.consonant = stod2(tokenList[2]);
        res.cutoff = stod2(tokenList[3]);
        res.preUtterance = stod2(tokenList[4]);
        res.voiceOverlap = stod2(tokenList[5]);
        for (int i = 0; i < 5; ++i) {
            res.spellings[i] = tokenList[i + 1];
        }
        return res;
    }

    /// The original text \a spelling if it still parses to exactly \a value .
    ///
    /// Otherwise the shortest fixed-notation text that parses to \a value . The shortest form in
    /// general would use exponent notation for large values, and UTAU is not known to accept it.
    static std::string formatNumber(double value, const std::string &spelling) {
        const auto read = toDouble(spelling);
        if (read && *read == value) {
            return spelling;
        }
        char buffer[400];
        const auto result =
            std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::fixed);
        return std::string(buffer, result.ptr);
    }

    static std::string formatEntry(const OtoEntry &entry) {
        const double numbers[] = {
            entry.offset, entry.consonant, entry.cutoff, entry.preUtterance, entry.voiceOverlap,
        };
        std::string out = entry.fileName;
        out += EQUAL;
        out += entry.alias;
        for (int i = 0; i < 5; ++i) {
            out += COMMA;
            out += formatNumber(numbers[i], entry.spellings[i]);
        }
        return out;
    }

    OtoIni::OtoIni() = default;

    bool OtoIni::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool OtoIni::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    bool OtoIni::read(std::string_view text) {
        std::string_view line;
        while (takeLine(text, line)) {
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

    std::string OtoIni::write() const {
        std::string out;
        for (const auto &item : contents) {
            for (const auto &entry : item.second) {
                out += formatEntry(entry);
                out += LINE_END;
            }
        }
        return out;
    }

}