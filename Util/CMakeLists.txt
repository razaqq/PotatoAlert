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
        Log.cpp
        Log.hpp
        Palette.cpp
        Palette.hpp
        PotatoClient.cpp
        PotatoClient.hpp
        Process.hpp
        Screenshot.cpp
        Screenshot.hpp
        StatsParser.cpp
        StatsParser.hpp
        StringTable.cpp
        StringTable.hpp
        Time.hpp
        ../Updater/Updater.cpp
        ../Updater/Updater.hpp
        Version.cpp
        Version.hpp
        Zip.hpp
        Zip.cpp
)

if(WIN32)
    set(
            utils_src
            ${utils_src}
            File.win32.cpp
            Process.win32.cpp
            Time.win32.cpp
    )
endif(WIN32)

if(UNIX)
    set(
            utils_src
            ${utils_src}
            Time.linux.cpp
    )
endif(UNIX)

add_library(util STATIC ${utils_src})
target_link_libraries(util PRIVATE tinyxml2 zip)
target_link_libraries(util PUBLIC Qt5::Core Qt5::WebSockets Qt5::Widgets)

if(WIN32)
    target_link_libraries(util PRIVATE Version)
endif(WIN32)