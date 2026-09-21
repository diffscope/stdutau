#include <string>

#include <stdutau/plugintxt.h>

#include <boost/test/unit_test.hpp>

using namespace utau;

BOOST_AUTO_TEST_SUITE(test_plugintxt)

namespace {

    PluginTxt parse(const std::string &text) {
        PluginTxt plugin;
        BOOST_REQUIRE(plugin.read(text));
        return plugin;
    }

    std::string written(const PluginTxt &plugin) {
        return plugin.write();
    }

    // What the library writes, whatever the input used. A UTAU file is CRLF on every platform.
    std::string crlf(const std::string &text) {
        std::string out;
        for (char c : text) {
            if (c == '\n') {
                out += '\r';
            }
            out += c;
        }
        return out;
    }

}

BOOST_AUTO_TEST_CASE(test_read) {
    auto plugin = parse("name=test\n"
                        "execute=test.exe\n"
                        "shell=use\n"
                        "ustversion=1.20\n"
                        "notes=all\n");

    BOOST_CHECK_EQUAL(plugin.name, "test");
    BOOST_CHECK_EQUAL(plugin.execute, "test.exe");
    BOOST_CHECK_EQUAL(plugin.shell, "use");
    BOOST_CHECK_EQUAL(plugin.ustVersion, "1.20");
    BOOST_REQUIRE(plugin.notes.has_value());
    BOOST_CHECK_EQUAL(*plugin.notes, "all");
    BOOST_CHECK(plugin.extraLines.empty());
}

// The two entries every plugin has. The pair out of a real install carries nothing else.
BOOST_AUTO_TEST_CASE(test_the_two_required_entries_are_enough) {
    auto plugin = parse("name=\xb2\xe5\xbc\xfe\n"
                        "execute=work.exe");

    BOOST_CHECK_EQUAL(plugin.name, "\xb2\xe5\xbc\xfe");
    BOOST_CHECK_EQUAL(plugin.execute, "work.exe");
    BOOST_CHECK(plugin.shell.empty());
    BOOST_CHECK(!plugin.notes.has_value());
}

// Having the entry at all is what hands the plugin the whole track, whatever the value, which is
// why an empty one is not the same as none.
BOOST_AUTO_TEST_CASE(test_notes_is_told_apart_from_no_notes) {
    BOOST_CHECK(!parse("name=test\n").notes.has_value());

    auto empty = parse("notes=\n");
    BOOST_REQUIRE(empty.notes.has_value());
    BOOST_CHECK_EQUAL(*empty.notes, "");
    BOOST_CHECK_EQUAL(written(empty), crlf("notes=\n"));
}

BOOST_AUTO_TEST_CASE(test_an_unknown_entry_is_kept) {
    auto plugin = parse("name=test\n"
                        "charset=Shift_JIS\n");

    BOOST_REQUIRE_EQUAL(plugin.extraLines.size(), 1);
    BOOST_CHECK_EQUAL(plugin.extraLines.at(0), "charset=Shift_JIS");
}

BOOST_AUTO_TEST_CASE(test_crlf_reads_the_same_as_lf) {
    BOOST_CHECK_EQUAL(parse("execute=test.exe\r\n").execute, "test.exe");
}

BOOST_AUTO_TEST_CASE(test_write_reads_back_the_same) {
    const std::string text = "name=test\n"
                             "execute=test.exe\n"
                             "shell=use\n"
                             "ustversion=1.20\n"
                             "notes=all\n"
                             "charset=Shift_JIS\n";

    BOOST_CHECK_EQUAL(written(parse(text)), crlf(text));
}

BOOST_AUTO_TEST_SUITE_END()
