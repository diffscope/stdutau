#include "plugintxt.h"

#include <fstream>

#include "utaconst.h"
#include "utautils.h"

namespace utau {

    PluginTxt::PluginTxt() = default;

    bool PluginTxt::load(const std::filesystem::path &path) {
        std::ifstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const std::string text((std::istreambuf_iterator<char>(fs)),
                               std::istreambuf_iterator<char>());
        return read(text);
    }

    bool PluginTxt::save(const std::filesystem::path &path) const {
        std::ofstream fs(path, std::ios::binary);
        if (!fs.is_open())
            return false;
        const auto text = write();
        fs.write(text.data(), std::streamsize(text.size()));
        return fs.good();
    }

    bool PluginTxt::read(std::string_view text) {
        std::string_view line;
        while (takeLine(text, line)) {
            const auto eq = line.find('=');
            if (eq == std::string_view::npos) {
                extraLines.emplace_back(line);
                continue;
            }

            const auto key = line.substr(0, eq);
            const auto value = line.substr(eq + 1);
            if (key == KEY_NAME_PLUGIN_NAME) {
                name = value;
            } else if (key == KEY_NAME_PLUGIN_EXECUTE) {
                execute = value;
            } else if (key == KEY_NAME_PLUGIN_SHELL) {
                shell = value;
            } else if (key == KEY_NAME_PLUGIN_UST_VERSION) {
                ustVersion = value;
            } else if (key == KEY_NAME_PLUGIN_NOTES) {
                notes = std::string(value);
            } else {
                extraLines.emplace_back(line);
            }
        }
        return true;
    }

    std::string PluginTxt::write() const {
        std::string out;

        const auto entry = [&out](const char *key, const std::string &value) {
            if (value.empty())
                return;
            out += key;
            out += '=';
            out += value;
            out += LINE_END;
        };

        entry(KEY_NAME_PLUGIN_NAME, name);
        entry(KEY_NAME_PLUGIN_EXECUTE, execute);
        entry(KEY_NAME_PLUGIN_SHELL, shell);
        entry(KEY_NAME_PLUGIN_UST_VERSION, ustVersion);

        // Written even when empty, since having the entry at all is what it says.
        if (notes) {
            out += KEY_NAME_PLUGIN_NOTES;
            out += '=';
            out += *notes;
            out += LINE_END;
        }

        for (const auto &line : extraLines) {
            out += line;
            out += LINE_END;
        }
        return out;
    }

}
