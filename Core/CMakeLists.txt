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

        src/ApplicationGuard.win32.cpp
        src/Directory.win32.cpp
        src/File.win32.cpp
        src/FileMapping.win32.cpp
        src/Process.win32.cpp
        src/Semaphore.win32.cpp
        src/Time.win32.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        core
        PRIVATE

        src/ApplicationGuard.linux.cpp
        src/Directory.linux.cpp
        src/File.linux.cpp
        src/FileMapping.linux.cpp
        src/Semaphore.linux.cpp
        src/Time.linux.cpp
    )
endif(UNIX)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

set_target_properties(core PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(core PRIVATE sqlite3 OpenSSL zip ZLIB::ZLIB)
target_link_libraries(core PUBLIC nlohmann_json Qt6::Core Qt6::Widgets spdlog tinyxml2)
target_include_directories(core PUBLIC include)

if(WIN32)
    target_link_libraries(core PRIVATE Version Win32 Ws2_32)
endif(WIN32)
