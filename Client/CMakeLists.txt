set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        client_src

        Game.cpp
        Game.hpp
        PotatoClient.cpp
        PotatoClient.hpp
        Serializer.cpp
        Serializer.hpp
        StatsParser.cpp
        StatsParser.hpp
        ../Updater/Updater.cpp
        ../Updater/Updater.hpp
)

if(WIN32)
    set(
            client_src
            ${client_src}
            Game.win32.cpp
    )
endif(WIN32)

if(UNIX)
    set(
            client_src
            ${client_src}
            Game.linux.cpp
    )
endif(UNIX)

find_package(Qt5 COMPONENTS Core Widgets WebSockets REQUIRED)

add_library(client STATIC ${client_src})
set_target_properties(client PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(client PRIVATE util replayparser)
target_link_libraries(client PUBLIC Qt5::Core Qt5::WebSockets Qt5::Widgets)
target_include_directories(client
        PUBLIC
        ${PROJECT_SOURCE_DIR}/Util
        ${PROJECT_SOURCE_DIR}/ReplayParser
        ${PROJECT_SOURCE_DIR}/ThirdParty/win32
)
if(WIN32)
    target_link_libraries(client PRIVATE Version)
endif(WIN32)
