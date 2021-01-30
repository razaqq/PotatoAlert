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
        CSVWriter.cpp
        Version.cpp
)

add_library(utils_lib STATIC ${utils_src})
target_link_libraries(
        utils_lib
        fmt::fmt-header-only
        tinyxml2
        zip
        Version
        Qt5::Core
        Qt5::WebSockets
        Qt5::Widgets)
