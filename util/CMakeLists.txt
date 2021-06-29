set(
        utils_src

        Config.cpp
        Config.hpp
        CSVWriter.cpp
        CSVWriter.hpp
        File.hpp
        Flags.hpp
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
        Time.hpp
        ../updater/Updater.hpp
        Version.cpp
        Version.hpp
)

if (WIN32)
    set(
            utils_src
            ${utils_src}
            File.win32.cpp
            Time.win32.cpp
            ../updater/Updater.win32.cpp
    )
endif (WIN32)

if (UNIX)
    set(
            utils_src
            ${utils_src}
            Time.linux.cpp
    )
endif (UNIX)

add_library(util STATIC ${utils_src})
target_link_libraries(
        util
        tinyxml2
        zip
        Version
        Qt5::Core
        Qt5::WebSockets
        Qt5::Widgets)
