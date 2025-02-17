add_library(PotatoAlert-Client STATIC)
add_library(PotatoAlert::Client ALIAS PotatoAlert-Client)

target_sources(PotatoAlert-Client
    PRIVATE
        src/Config.cpp
        src/DatabaseManager.cpp
        src/DirectoryWatcher.cpp
        src/Game.cpp
        src/Log.cpp
        src/PotatoClient.cpp
        src/ReplayAnalyzer.cpp
        src/Screenshot.cpp
        src/StatsParser.cpp
)

if(WIN32)
    target_sources(PotatoAlert-Client
        PRIVATE
            src/Game.win32.cpp
            src/SysInfo.win32.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(PotatoAlert-Client
        PRIVATE
            src/Game.linux.cpp
            src/SysInfo.linux.cpp
    )
endif(UNIX)

# have to add some headers to sources for qt AUTOMOC
target_sources(PotatoAlert-Client
    PRIVATE
        include/Client/Config.hpp
        include/Client/DirectoryWatcher.hpp
        include/Client/PotatoClient.hpp
        include/Client/ReplayAnalyzer.hpp
)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)
find_package(ctre REQUIRED)
find_package(fmt REQUIRED)
find_package(tinyxml2 REQUIRED)

set_target_properties(PotatoAlert-Client
    PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE
        AUTOUIC TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        DEBUG_POSTFIX "d"
)
target_link_libraries(PotatoAlert-Client
    PUBLIC
        PotatoAlert::Core
        PotatoAlert::GameFileUnpack
        PotatoAlert::ReplayParser
        fmt::fmt
        Qt::Core
        Qt::Network
        Qt::Widgets
    PRIVATE
        ctre::ctre
        tinyxml2::tinyxml2
        Win32
)
target_include_directories(PotatoAlert-Client
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Resources
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
include(CompilerFlags)
SetCompilerFlags(PotatoAlert-Client)

if(WIN32)
    target_link_libraries(PotatoAlert-Client PRIVATE Version)
endif(WIN32)
