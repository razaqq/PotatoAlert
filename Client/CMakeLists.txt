set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(client STATIC)

target_sources(
    client
    PRIVATE

    src/Config.cpp
    src/Game.cpp
    src/MatchHistory.cpp
    src/PotatoClient.cpp
    src/ReplayAnalyzer.cpp
    src/StatsParser.cpp
    src/StringTable.cpp
)

if(WIN32)
    target_sources(
        client
        PRIVATE

        src/Game.win32.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        client
        PRIVATE

        src/Game.linux.cpp
    )
endif(UNIX)

# have to add some headers to sources for qt AUTOMOC
target_sources(
    client
    PRIVATE

    include/Client/Config.hpp
    include/Client/PotatoClient.hpp
    include/Client/ReplayAnalyzer.hpp
)

find_package(Qt5 COMPONENTS Core Widgets WebSockets REQUIRED)

set_target_properties(client PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(client PRIVATE tinyxml2 Win32)
target_link_libraries(client PUBLIC core Qt5::Core Qt5::WebSockets Qt5::Widgets replayparser)
target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/Resources include)

if(WIN32)
    target_link_libraries(client PRIVATE Version)
endif(WIN32)
