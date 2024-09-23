set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(Client STATIC)

target_sources(
    Client
    PRIVATE

    src/Config.cpp
    src/DatabaseManager.cpp
    src/Game.cpp
    src/PotatoClient.cpp
    src/ReplayAnalyzer.cpp
    src/StatsParser.cpp
)

if(WIN32)
    target_sources(
        Client
        PRIVATE

        src/Game.win32.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        Client
        PRIVATE

        src/Game.linux.cpp
    )
endif(UNIX)

# have to add some headers to sources for qt AUTOMOC
target_sources(
    Client
    PRIVATE

    include/Client/Config.hpp
    include/Client/PotatoClient.hpp
    include/Client/ReplayAnalyzer.hpp
)

find_package(Qt6 COMPONENTS Core Network Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Network Widgets REQUIRED)
endif()

set_target_properties(Client PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(Client PRIVATE ctre tinyxml2 Win32)
target_link_libraries(Client PUBLIC Core GameFileUnpack ReplayParser fmt::fmt Qt::Core Qt::Network Qt::Widgets)
target_include_directories(Client PUBLIC ${PROJECT_SOURCE_DIR}/Resources include)
include(CompilerFlags)
SetCompilerFlags(Client)

if(WIN32)
    target_link_libraries(Client PRIVATE Version)
endif(WIN32)
