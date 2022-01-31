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
            Process.win32.cpp
            Time.win32.cpp
    )
endif(WIN32)

if(UNIX)
    set(
            core_src
            ${core_src}
            Time.linux.cpp
    )
endif(UNIX)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/openssl ${CMAKE_BINARY_DIR}/ThirdParty/openssl)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/tinyxml2 ${CMAKE_BINARY_DIR}/ThirdParty/tinyxml2)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/sqlite3 ${CMAKE_BINARY_DIR}/ThirdParty/sqlite3)
set(CMAKE_DISABLE_TESTING ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/zip ${CMAKE_BINARY_DIR}/ThirdParty/zip)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/zlib ${CMAKE_BINARY_DIR}/ThirdParty/zlib)
target_compile_options(zlib PRIVATE -Wno-deprecated)
target_compile_options(zlibstatic PRIVATE -Wno-deprecated)

add_library(core STATIC ${core_src})
set_target_properties(core PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
target_link_libraries(core PRIVATE sqlite3 tinyxml2 zip ZLIB::ZLIB OpenSSL)
target_link_libraries(core PUBLIC Qt5::Core Qt5::Widgets)
target_include_directories(
        core
        PUBLIC
        ${PROJECT_SOURCE_DIR}/Resources
        SYSTEM
        ${PROJECT_SOURCE_DIR}/ThirdParty/nlohmann_json/include
        ${PROJECT_SOURCE_DIR}/ThirdParty/spdlog/include
        ${PROJECT_SOURCE_DIR}/ThirdParty/sqlite3/include
        ${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/include
        ${PROJECT_SOURCE_DIR}/ThirdParty/win32
        ${PROJECT_SOURCE_DIR}/ThirdParty/zip/src
        ${PROJECT_SOURCE_DIR}/ThirdParty/zlib
)

if(WIN32)
    target_link_libraries(core PRIVATE Version)
endif(WIN32)