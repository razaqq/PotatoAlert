set(
        utils_src

        Config.cpp
        Game.cpp
        Logger.cpp
        Palette.cpp
        PotatoClient.cpp
        StatsParser.cpp
        ../updater/Updater.cpp
        StringTable.cpp
        CSVWriter.cpp
        Version.cpp
)

add_library(util STATIC ${utils_src})
target_link_libraries(
        util
        fmt::fmt-header-only
        tinyxml2
        zip
        Version
        Qt5::Core
        Qt5::WebSockets
        Qt5::Widgets)
