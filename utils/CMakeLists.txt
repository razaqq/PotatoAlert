set(
        utils_src

        Config.cpp
        Game.cpp
        Logger.cpp
        Palette.cpp
        PotatoClient.cpp
        StatsParser.cpp
        Updater.cpp
        StringTable.cpp
)
set(CMAKE_CXX_FLAGS "/EHsc")

add_library(utilslib ${utils_src})
target_link_libraries(
        utilslib
        debug fmtd optimized fmt
        debug tinyxml2d optimized tinyxml2
        Qt5::Core
        Qt5::WebSockets
        Qt5::Widgets)
