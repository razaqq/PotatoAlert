cmake_minimum_required(VERSION 3.17)
project(Core)

add_library(PotatoAlert-Core STATIC)
add_library(PotatoAlert::Core ALIAS PotatoAlert-Core)

target_sources(PotatoAlert-Core
    PRIVATE
        src/Blowfish.cpp
        src/Directory.cpp
        src/Log.cpp
        src/PeFileVersion.cpp
        src/PeReader.cpp
        src/Python.cpp
        src/Sha1.cpp
        src/Sha256.cpp
        src/String.cpp
        src/Sqlite.cpp
        src/Time.cpp
        src/ThreadPool.cpp
        src/Version.cpp
        src/Zip.cpp
        src/Zlib.cpp
)

if(WIN32)
    target_sources(PotatoAlert-Core
        PRIVATE
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
    )
endif(WIN32)

if(UNIX)
    target_sources(PotatoAlert-Core
        PRIVATE
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
endif(UNIX)

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

find_package(Python 3.13 EXACT REQUIRED COMPONENTS Development)
target_include_directories(PotatoAlert-Core PRIVATE ${Python_INCLUDE_DIRS})
target_link_directories(PotatoAlert-Core PUBLIC ${Python_LIBRARY_DIRS})

set_target_properties(PotatoAlert-Core
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        DEBUG_POSTFIX "d"
)
target_link_libraries(PotatoAlert-Core
    PUBLIC
        fmt::fmt
        rapidjson
        spdlog::spdlog_header_only
        tinyxml2::tinyxml2
    PRIVATE
        ctre::ctre
        date::date
        openssl::openssl
        SQLite::SQLite3
        zip::zip
        ZLIB::ZLIB
)
target_include_directories(PotatoAlert-Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
include(CompilerFlags)
SetCompilerFlags(PotatoAlert-Core)

if(WIN32)
    target_link_libraries(PotatoAlert-Core
        PRIVATE
            Version
            Win32
            Ws2_32
)
endif(WIN32)
