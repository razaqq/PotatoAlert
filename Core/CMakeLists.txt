set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        core_src

        Blowfish.cpp
        Config.cpp
        DirectoryWatcher.cpp
        Log.cpp
        Screenshot.cpp
        Sha1.cpp
        Sha256.cpp
        String.cpp
        StringTable.cpp
        Sqlite.cpp
        ThreadPool.cpp
        Version.cpp
        Zip.cpp
        Zlib.cpp
)

if(WIN32)
    set(
            core_src
            ${core_src}
            Directory.win32.cpp
            File.win32.cpp
            Mutex.win32.cpp
            Process.win32.cpp
            Time.win32.cpp
    )
endif(WIN32)

if(UNIX)
    set(
            core_src
            ${core_src}
            Mutex.linux.cpp
            Time.linux.cpp
    )
endif(UNIX)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_library(core STATIC ${core_src})
set_target_properties(core PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
target_link_libraries(core PRIVATE sqlite3 OpenSSL Win32 zip ZLIB::ZLIB)
target_link_libraries(core PUBLIC nlohmann_json Qt5::Core Qt5::Widgets spdlog tinyxml2)
target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/Resources)

if(WIN32)
    target_link_libraries(core PRIVATE Version Ws2_32)
endif(WIN32)
