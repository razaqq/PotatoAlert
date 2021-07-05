set(
        gui_src

        AboutWidget.cpp
        AboutWidget.hpp
        FramelessDialog.hpp
        MainWindow.cpp
        MainWindow.hpp
        NativeWindow.cpp
        NativeWindow.hpp
        TitleBar.cpp
        TitleBar.hpp

        MenuBar/MenuEntryButton.cpp
        MenuBar/MenuEntryButton.hpp
        MenuBar/VerticalMenuBar.cpp
        MenuBar/VerticalMenuBar.hpp

        SettingsWidget/FolderStatus.cpp
        SettingsWidget/FolderStatus.hpp
        SettingsWidget/HorizontalLine.cpp
        SettingsWidget/HorizontalLine.hpp
        SettingsWidget/SettingsChoice.cpp
        SettingsWidget/SettingsChoice.hpp
        SettingsWidget/SettingsSwitch.cpp
        SettingsWidget/SettingsSwitch.hpp
        SettingsWidget/SettingsWidget.cpp
        SettingsWidget/SettingsWidget.hpp

        StatsWidget/StatsHeader.cpp
        StatsWidget/StatsHeader.hpp
        StatsWidget/StatsTable.cpp
        StatsWidget/StatsTable.hpp
        StatsWidget/StatsTeamFooter.cpp
        StatsWidget/StatsTeamFooter.hpp
        StatsWidget/StatsWidget.cpp
        StatsWidget/StatsWidget.hpp

        ../Updater/UpdaterGui.cpp
        ../Updater/UpdaterGui.hpp
)

if(WIN32)
    set(
            gui_src
            ${gui_src}
            FramelessDialog.win32.cpp
    )
endif(WIN32)

add_library(gui STATIC ${gui_src})
target_link_libraries(gui PRIVATE util framelessHelper Qt5::Widgets)
