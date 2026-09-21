#include "charactertxt.h"

#include <fstream>

#include "utaconst.h"
#include "utautils.h"

namespace utau {

    CharacterTxt::CharacterTxt() = default;

    bool CharacterTxt::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool CharacterTxt::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    bool CharacterTxt::read(std::string_view text) {
        std::string_view line;
        while (takeLine(text, line)) {
            const auto eq = line.find('=');
            if (eq == std::string_view::npos) {
                extraLines.emplace_back(line);
                continue;
            }

            const auto key = line.substr(0, eq);
            const auto value = line.substr(eq + 1);
            if (key == KEY_NAME_CHAR_NAME) {
                name = value;
            } else if (key == KEY_NAME_CHAR_IMAGE) {
                image = value;
            } else if (key == KEY_NAME_CHAR_SAMPLE) {
                sample = value;
            } else if (key == KEY_NAME_CHAR_AUTHOR) {
                author = value;
            } else if (key == KEY_NAME_CHAR_WEB) {
                web = value;
            } else {
                extraLines.emplace_back(line);
            }
        }
        return true;
    }

    std::string CharacterTxt::write() const {
        std::string out;

        const auto entry = [&out](const char *key, const std::string &value) {
            if (value.empty())
                return;
            out += key;
            out += '=';
            out += value;
            out += LINE_END;
        };

        entry(KEY_NAME_CHAR_NAME, name);
        entry(KEY_NAME_CHAR_IMAGE, image);
        entry(KEY_NAME_CHAR_SAMPLE, sample);
        entry(KEY_NAME_CHAR_AUTHOR, author);
        entry(KEY_NAME_CHAR_WEB, web);

        for (const auto &line : extraLines) {
            out += line;
            out += LINE_END;
        }
        return out;
    }

}
