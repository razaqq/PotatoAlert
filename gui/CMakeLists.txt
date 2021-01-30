set(
        gui_src

        AboutWidget.cpp
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
        FramelessDialog.cpp)

add_library(gui_lib STATIC ${gui_src})
target_link_libraries(gui_lib utils_lib Qt5::Widgets)
