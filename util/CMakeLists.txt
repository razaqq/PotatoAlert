set(
        utils_src

        Config.cpp
        Config.hpp
        CSVWriter.cpp
        CSVWriter.hpp
        Game.cpp
        Game.hpp
        Logger.cpp
        Logger.hpp
        Palette.cpp
        Palette.hpp
        PotatoClient.cpp
        PotatoClient.hpp
        StatsParser.cpp
        StatsParser.hpp
        StringTable.cpp
        StringTable.hpp
        Time.cpp
        Time.hpp
        ../updater/Updater.cpp
        ../updater/Updater.hpp
        Version.cpp
        Version.hpp
)

add_library(util STATIC ${utils_src})
target_link_libraries(
        util
        tinyxml2
        zip
        Version
        Qt5::Core
        Qt5::WebSockets
        Qt5::Widgets)
