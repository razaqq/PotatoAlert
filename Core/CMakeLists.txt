set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(Core STATIC)

target_sources(
    Core
    PRIVATE

    src/Blowfish.cpp
    src/Directory.cpp
    src/DirectoryWatcher.cpp
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
)

# have to add some headers to sources for qt AUTOMOC
target_sources(
    Core
    PRIVATE

    include/Core/DirectoryWatcher.hpp
)

if(WIN32)
    target_sources(
        Core
        PRIVATE

        src/ApplicationGuard.win32.cpp
        src/Directory.win32.cpp
        src/Encoding.win32.cpp
        src/File.win32.cpp
        src/FileMapping.win32.cpp
        src/Process.win32.cpp
        src/Semaphore.win32.cpp
        src/Time.win32.cpp
        src/Xml.win32.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        Core
        PRIVATE

        src/ApplicationGuard.linux.cpp
        src/Directory.linux.cpp
        src/Encoding.linux.cpp
        src/File.linux.cpp
        src/FileMapping.linux.cpp
        src/Process.linux.cpp
        src/Semaphore.linux.cpp
        src/Time.linux.cpp
        src/Xml.linux.cpp
    )
endif(UNIX)

find_package(Qt6 COMPONENTS Core Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
endif()

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

set_target_properties(Core PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(Core PRIVATE ctre::ctre date::date SQLite::SQLite3 openssl::openssl zip::zip ZLIB::ZLIB)
target_link_libraries(Core PUBLIC Qt::Core Qt::Widgets rapidjson fmt::fmt spdlog::spdlog_header_only tinyxml2::tinyxml2)
target_include_directories(Core PUBLIC include)
include(CompilerFlags)
SetCompilerFlags(Core)

if(WIN32)
    target_link_libraries(Core PRIVATE Version Win32 Ws2_32)
endif(WIN32)
