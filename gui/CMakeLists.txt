set(
        gui_src

        AboutWidget.cpp
        HelpWidget.cpp
        MainWindow.cpp
        NativeWindow.cpp
        TitleBar.cpp
        MenuBar/MenuEntry.cpp
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
)

add_library(guilib ${gui_src})
target_link_libraries(guilib utilslib Qt5::Widgets)
