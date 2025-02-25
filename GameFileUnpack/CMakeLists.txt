cmake_minimum_required(VERSION 3.17)
project(GameFileUnpack)

if(WIN32)
    set(LIB_TYPE SHARED)
elseif(LINUX)
    set(LIB_TYPE STATIC)
else()
    message(FATAL_ERROR "PotatoAlert: Unsupported OS")
endif()
message(STATUS "PotatoAlert: Building GameFileUnpack as ${LIB_TYPE} library")

pa_add_library(
    PotatoAlert::GameFileUnpack
    ${LIB_TYPE}
    HEADERS
        include/GameFileUnpack/GameFileUnpack.hpp
    SOURCES
        src/GameFileUnpack.cpp
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS TRUE
        OUTPUT_NAME "${PROJECT_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        DEBUG_POSTFIX "d"
    HEADER_INCLUDE_DIRECTORIES
        include
    HEADER_DEPENDENCIES
        PotatoAlert::Core
    TEST_SOURCES
        Test/GameFileUnpackTest.cpp
    TEST_PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
    TEST_WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/Data"
)
pa_install_target(
    PotatoAlert::GameFileUnpack
    COMPONENT ${PROJECT_NAME}
    RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
pa_archive_component(${PROJECT_NAME})
