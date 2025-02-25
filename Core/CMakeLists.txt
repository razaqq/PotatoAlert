cmake_minimum_required(VERSION 3.24)
project(Core)

find_package(ctre REQUIRED)
find_package(date REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(zip REQUIRED)
find_package(ZLIB REQUIRED)

pa_add_library(
    PotatoAlert::Core
    HEADERS
        include/Core/ApplicationGuard.hpp
        include/Core/AsciiTable.hpp
        include/Core/Blowfish.hpp
        include/Core/ByteReader.hpp
        include/Core/Bytes.hpp
        include/Core/Concepts.hpp
        include/Core/Defer.hpp
        include/Core/Directory.hpp
        include/Core/Encoding.hpp
        include/Core/File.hpp
        include/Core/FileMagic.hpp
        include/Core/FileMapping.hpp
        include/Core/Flags.hpp
        include/Core/Format.hpp
        include/Core/Hash.hpp
        include/Core/Instrumentor.hpp
        include/Core/Json.hpp
        include/Core/Log.hpp
        include/Core/Math.hpp
        include/Core/PeFileVersion.hpp
        include/Core/PeReader.hpp
        include/Core/Preprocessor.hpp
        include/Core/Process.hpp
        include/Core/Result.hpp
        include/Core/Semaphore.hpp
        include/Core/Sha1.hpp
        include/Core/Sha256.hpp
        include/Core/Singleton.hpp
        include/Core/Sqlite.hpp
        include/Core/StandardPaths.hpp
        include/Core/String.hpp
        include/Core/ThreadPool.hpp
        include/Core/Time.hpp
        include/Core/TypeTraits.hpp
        include/Core/Version.hpp
        include/Core/Xml.hpp
        include/Core/Zip.hpp
        include/Core/Zlib.hpp
    SOURCES
        src/Blowfish.cpp
        src/Directory.cpp
        src/Log.cpp
        src/PeFileVersion.cpp
        src/PeReader.cpp
        src/Sha1.cpp
        src/Sha256.cpp
        src/String.cpp
        src/Sqlite.cpp
        src/Time.cpp
        src/ThreadPool.cpp
        src/Version.cpp
        src/Zip.cpp
        src/Zlib.cpp
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS TRUE
        OUTPUT_NAME "${PROJECT_NAME}"
        DEBUG_POSTFIX "d"
    HEADER_INCLUDE_DIRECTORIES
        include
    HEADER_DEPENDENCIES
        fmt::fmt
        rapidjson
        spdlog::spdlog_header_only
        tinyxml2::tinyxml2
    SOURCE_DEPENDENCIES
        ctre::ctre
        date::date
        openssl::openssl
        SQLite::SQLite3
        zip::zip
        ZLIB::ZLIB
    MANUAL_DEPENDENCIES
        PotatoAlert-Resources
    SOURCE_DEFINITIONS
        PA_LOOKUP_URL="${PA_LOOKUP_URL}"
        PA_SUBMIT_URL="${PA_SUBMIT_URL}"
    TEST_SOURCES
        Test/CoreTest.cpp
    TEST_PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
    TEST_WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/Data"
)
pa_configure(
    PotatoAlert::Core
    PLATFORM Windows
    SOURCES
        src/ApplicationGuard.win32.cpp
        src/Directory.win32.cpp
        src/Encoding.win32.cpp
        src/File.win32.cpp
        src/FileMapping.win32.cpp
        src/Process.win32.cpp
        src/Semaphore.win32.cpp
        src/StandardPaths.win32.cpp
        src/Time.win32.cpp
        src/Xml.win32.cpp
    SOURCE_DEPENDENCIES
        Version
        Win32
        Ws2_32
)
pa_configure(
    PotatoAlert::Core
    PLATFORM Linux
    SOURCES
        src/ApplicationGuard.linux.cpp
        src/Directory.linux.cpp
        src/Encoding.linux.cpp
        src/File.linux.cpp
        src/FileMapping.linux.cpp
        src/Process.linux.cpp
        src/Semaphore.linux.cpp
        src/StandardPaths.linux.cpp
        src/Time.linux.cpp
        src/Xml.linux.cpp
)
pa_install_target(
    PotatoAlert::Core
    COMPONENT ${PROJECT_NAME}
    RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
pa_archive_component(${PROJECT_NAME})
