set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(core STATIC)

target_sources(
        core
        PRIVATE

        src/Blowfish.cpp
        src/DirectoryWatcher.cpp
        src/Log.cpp
        src/Screenshot.cpp
        src/Sha1.cpp
        src/Sha256.cpp
        src/String.cpp
        src/Sqlite.cpp
        src/ThreadPool.cpp
        src/Version.cpp
        src/Zip.cpp
        src/Zlib.cpp
)

# have to add some headers to sources for qt AUTOMOC
target_sources(
        core
        PRIVATE

        include/Core/DirectoryWatcher.hpp
)

if(WIN32)
    target_sources(
            core
            PRIVATE

            src/Directory.win32.cpp
            src/File.win32.cpp
            src/Mutex.win32.cpp
            src/Process.win32.cpp
            src/Time.win32.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
            core
            PRIVATE

            src/Mutex.linux.cpp
            src/Time.linux.cpp
    )
endif(UNIX)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set_target_properties(core PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(core PRIVATE sqlite3 OpenSSL Win32 zip ZLIB::ZLIB)
target_link_libraries(core PUBLIC nlohmann_json Qt5::Core Qt5::Widgets spdlog tinyxml2)
target_include_directories(core PUBLIC include)

if(WIN32)
    target_link_libraries(core PRIVATE Version Ws2_32)
endif(WIN32)
