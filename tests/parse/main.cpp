#include <iostream>
#include <string>

#include <stdutau/ustfile.h>

static inline std::string standardError(int code = errno) {
    return std::error_code(code, std::generic_category()).message();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: tst_parse <ust> <out>\n");
        return 0;
    }

    std::string_view fileName = argv[1];
    std::string_view output = argv[2];
    Utau::UstFile ust;
    if (!ust.load(fileName)) {
        std::cout << standardError();
        return -1;
    }

    if (!ust.save(output)) {
        std::cout << standardError();
        return -1;
    }
    return 0;
}