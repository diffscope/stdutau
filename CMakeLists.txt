cmake_minimum_required(VERSION 3.16)

project(stdutau VERSION 0.0.1.1 LANGUAGES CXX)

option(STDUTAU_BUILD_STATIC "Build static library" on)
option(STDUTAU_INSTALL "Install library" on)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(STDUTAU_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

set(STDUTAU_INSTALL_NAME ${PROJECT_NAME})

add_subdirectory(src)