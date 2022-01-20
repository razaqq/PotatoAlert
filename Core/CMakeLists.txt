set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        core_src

        AsciiTable.hpp
        Blowfish.cpp
        Blowfish.hpp
        Bytes.hpp
        Config.cpp
        Config.hpp
        Directory.hpp
        File.hpp
        Flags.hpp
        Instrumentor.hpp
        Log.cpp
        Log.hpp
        Process.hpp
        Screenshot.cpp
        Screenshot.hpp
        Sha1.cpp
        Sha1.hpp
        String.cpp
        String.hpp
        StringTable.cpp
        StringTable.hpp
        Sqlite.cpp
        Sqlite.hpp
        Unpickler.cpp
        Unpickler.hpp
        ThreadPool.cpp
        ThreadPool.hpp
        Time.hpp
        Version.cpp
        Version.hpp
        Zip.hpp
        Zip.cpp
        Zlib.cpp
        Zlib.hpp
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

add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/tinyxml2 ${CMAKE_BINARY_DIR}/ThirdParty/tinyxml2)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/sqlite3 ${CMAKE_BINARY_DIR}/ThirdParty/sqlite3)
set(CMAKE_DISABLE_TESTING ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/zip ${CMAKE_BINARY_DIR}/ThirdParty/zip)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/zlib ${CMAKE_BINARY_DIR}/ThirdParty/zlib)
target_compile_options(zlib PRIVATE -Wno-deprecated)
target_compile_options(zlibstatic PRIVATE -Wno-deprecated)

add_library(core STATIC ${core_src})
set_target_properties(core PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
target_link_libraries(core PRIVATE sqlite3 tinyxml2 zip ZLIB::ZLIB)
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
