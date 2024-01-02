if(STDUTAU_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC STDUTAU_STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE STDUTAU_LIBRARY)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

file(GLOB_RECURSE _src *.h *.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${_src})

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
)

target_include_directories(${PROJECT_NAME} PRIVATE .)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Add platform specific
if(WIN32)
    set(RC_DESCRIPTION "C++ Standard UTAU Library")
    set(RC_COPYRIGHT "Copyright (C) 2020-2024 SineSriker")
    include("../../cmake/winrc.cmake")
endif()

if(STDUTAU_INSTALL)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    install(TARGETS ${PROJECT_NAME}
        EXPORT ${STDUTAU_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
    )

    # Install headers
    file(GLOB _headers *.h)
    install(FILES ${_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${STDUTAU_INSTALL_NAME})
endif()

if(STDUTAU_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${STDUTAU_INSTALL_NAME})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${STDUTAU_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/${STDUTAU_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${STDUTAU_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${STDUTAU_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${STDUTAU_INSTALL_NAME}ConfigVersion.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT ${STDUTAU_INSTALL_NAME}Targets
        FILE "${STDUTAU_INSTALL_NAME}Targets.cmake"
        NAMESPACE ${STDUTAU_INSTALL_NAME}::
        DESTINATION ${_install_dir}
    )
endif()