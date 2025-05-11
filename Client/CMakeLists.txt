cmake_minimum_required(VERSION 3.24)
project(Client)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)
find_package(ctre REQUIRED)
find_package(fmt REQUIRED)
find_package(tinyxml2 REQUIRED)

pa_add_library(
    PotatoAlert::Client
    STATIC
    SOURCES
        src/Config.cpp
        src/DatabaseManager.cpp
        src/DirectoryWatcher.cpp
        src/Game.cpp
        src/Log.cpp
        src/PotatoClient.cpp
        src/ReplayAnalyzer.cpp
        src/Screenshot.cpp
        src/StatsParser.cpp

        # have to add some headers to sources for qt AUTOMOC
        include/Client/Config.hpp
        include/Client/DirectoryWatcher.hpp
        include/Client/PotatoClient.hpp
        include/Client/ReplayAnalyzer.hpp
    PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE
        AUTOUIC TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS TRUE
        DEBUG_POSTFIX "d"
    HEADER_INCLUDE_DIRECTORIES
        ${PROJECT_SOURCE_DIR}/Resources
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}
    HEADER_DEPENDENCIES
        PotatoAlert::Core
        PotatoAlert::GameFileUnpack
        PotatoAlert::ReplayParser
        fmt::fmt
        Qt::Core
        Qt::Network
        Qt::Widgets
    SOURCE_DEPENDENCIES
        ctre::ctre
        tinyxml2::tinyxml2
    MANUAL_DEPENDENCIES
        PotatoAlert-StringTable
    TEST_SOURCES
        Test/ClientTest.cpp
    TEST_PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
    TEST_WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/Data"
)
pa_configure(
    PotatoAlert::Client
    PLATFORM Windows
    SOURCES
        src/Game.win32.cpp
        src/SysInfo.win32.cpp
        src/Updater.win32.cpp
        include/Client/Updater.hpp
    SOURCE_DEPENDENCIES
        Dwmapi
        Version
        Win32
)
pa_configure(
    PotatoAlert::Client
    PLATFORM Linux
    SOURCES
        src/Game.linux.cpp
        src/SysInfo.linux.cpp
)
