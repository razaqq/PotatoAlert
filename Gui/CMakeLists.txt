set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(gui STATIC)

target_sources(
        gui
        PRIVATE

        src/AboutWidget.cpp
        src/MainWindow.cpp
        src/MatchHistory.cpp
        src/NativeWindow.cpp
        src/Palette.cpp
        src/TitleBar.cpp
        src/Updater.cpp

        src/MenuBar/MenuEntryButton.cpp
        src/MenuBar/VerticalMenuBar.cpp

        src/SettingsWidget/FolderStatus.cpp
        src/SettingsWidget/HorizontalLine.cpp
        src/SettingsWidget/SettingsChoice.cpp
        src/SettingsWidget/SettingsSwitch.cpp
        src/SettingsWidget/SettingsWidget.cpp

        src/StatsWidget/StatsHeader.cpp
        src/StatsWidget/StatsTable.cpp
        src/StatsWidget/StatsTeamFooter.cpp
        src/StatsWidget/StatsWidget.cpp
)

# have to add some headers to sources for qt AUTOMOC
target_sources(
        gui
        PRIVATE

        include/Gui/MatchHistory.hpp
        include/Gui/NativeWindow.hpp
        include/Gui/MenuBar/VerticalMenuBar.hpp
        include/Gui/SettingsWidget/SettingsSwitch.hpp
        include/Gui/SettingsWidget/SettingsWidget.hpp
        include/Gui/StatsWidget/StatsHeader.hpp
        include/Gui/TitleBar.hpp
)

if(WIN32)
    target_sources(
            gui
            PRIVATE

            src/FramelessDialog.win32.cpp
            src/NativeWindow.win32.cpp
    )
endif(WIN32)

find_package(Qt5 COMPONENTS Widgets REQUIRED)


target_link_libraries(gui PRIVATE Dwmapi FramelessHelper Qt5::Widgets Win32)
target_link_libraries(gui PUBLIC client core updater)
target_include_directories(gui PUBLIC include ${CMAKE_BINARY_DIR})
