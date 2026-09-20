project(test_auto LANGUAGES CXX)

find_package(Boost CONFIG COMPONENTS unit_test_framework)

# Fatal rather than a warning and a return. Somebody who set STDUTAU_BUILD_TESTS=ON and got a
# green build with no tests in it has been told nothing.
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

# One entry rather than one per suite. The whole binary runs in well under a second, and the
# process starts would cost more than the suites they would be splitting.
#
# Note that ctest exits 0 when it finds no tests at all, so anything running this in anger wants
# --no-tests=error, or it is back to a green run of nothing.
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

set_tests_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 120)
