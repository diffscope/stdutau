project(test_auto LANGUAGES CXX)

find_package(Boost CONFIG COMPONENTS unit_test_framework)

# Fatal rather than a warning and a return, because a successful build without tests after
# STDUTAU_BUILD_TESTS=ON would report nothing.
if(NOT Boost_FOUND)
    message(FATAL_ERROR
        "STDUTAU_BUILD_TESTS is on but Boost.Test was not found. Install it, or point CMake at it "
        "with -DCMAKE_PREFIX_PATH, or turn the tests off with -DSTDUTAU_BUILD_TESTS=OFF.")
endif()

file(GLOB _src *.cpp)
add_executable(${PROJECT_NAME} ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::unit_test_framework)
target_link_libraries(${PROJECT_NAME} PRIVATE stdutau::stdutau)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# One entry rather than one per suite. The entire binary runs in well under a second, and
# separate process starts would cost more than the suites themselves.
#
# ctest exits with 0 if it finds no tests, so automated runs must pass --no-tests=error to avoid
# a successful run that tested nothing.
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

set_tests_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 120)
