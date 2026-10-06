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

    namespace {

        // Returns the member that holds the entry on \a line, or nullptr if the line is not an
        // entry with a dedicated member.
        std::string CharacterTxt::*memberOf(std::string_view line) {
            const auto eq = line.find('=');
            if (eq == std::string_view::npos) {
                return nullptr;
            }
            const auto key = line.substr(0, eq);
            if (key == KEY_NAME_CHAR_NAME) {
                return &CharacterTxt::name;
            }
            if (key == KEY_NAME_CHAR_IMAGE) {
                return &CharacterTxt::image;
            }
            if (key == KEY_NAME_CHAR_SAMPLE) {
                return &CharacterTxt::sample;
            }
            if (key == KEY_NAME_CHAR_AUTHOR) {
                return &CharacterTxt::author;
            }
            if (key == KEY_NAME_CHAR_WEB) {
                return &CharacterTxt::web;
            }
            return nullptr;
        }

    }

    bool CharacterTxt::read(std::string_view text) {
        std::vector<std::string_view> lines;
        std::string_view line;
        while (takeLineKeepingEmpty(text, line)) {
            lines.push_back(line);
        }

        // An empty line before the last entry is dropped, because write() writes the entries
        // before the other lines and the empty line would not remain in its position. The lines
        // after the last entry are kept unchanged, empty lines included, so that the text after
        // the entries is written back as it was read.
        std::size_t lastEntry = 0;
        bool hasEntry = false;
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (memberOf(lines[i])) {
                lastEntry = i;
                hasEntry = true;
            }
        }

        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (const auto member = memberOf(lines[i])) {
                this->*member = lines[i].substr(lines[i].find('=') + 1);
            } else if (!lines[i].empty() || !hasEntry || i > lastEntry) {
                extraLines.emplace_back(lines[i]);
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
