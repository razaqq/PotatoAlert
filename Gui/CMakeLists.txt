set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        gui_src

        AboutWidget.cpp
        MainWindow.cpp
        MatchHistory.cpp
        NativeWindow.cpp
        Palette.cpp
        TitleBar.cpp
        
        MenuBar/MenuEntryButton.cpp
        MenuBar/VerticalMenuBar.cpp

        SettingsWidget/FolderStatus.cpp
        SettingsWidget/HorizontalLine.cpp
        SettingsWidget/SettingsChoice.cpp
        SettingsWidget/SettingsSwitch.cpp
        SettingsWidget/SettingsWidget.cpp

        StatsWidget/StatsHeader.cpp
        StatsWidget/StatsTable.cpp
        StatsWidget/StatsTeamFooter.cpp
        StatsWidget/StatsWidget.cpp

        ../Updater/UpdaterGui.cpp
)

if(WIN32)
    set(
            gui_src
            ${gui_src}
            FramelessDialog.win32.cpp
            NativeWindow.win32.cpp
    )
endif(WIN32)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(gui STATIC ${gui_src})

set_target_properties(gui PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
target_link_libraries(gui PRIVATE client core framelessHelper Qt5::Widgets Win32)
target_include_directories(
        gui
        PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)
