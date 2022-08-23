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
    src/QuestionDialog.cpp
    src/TitleBar.cpp

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
        src/Updater.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        gui
        PRIVATE

        src/FramelessDialog.linux.cpp
        src/NativeWindow.linux.cpp
    )
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED)

target_link_libraries(gui PRIVATE FramelessHelperWidgets Qt6::Widgets)

if(WIN32)
    target_link_libraries(gui PRIVATE Dwmapi Win32 updater)
endif()

target_link_libraries(gui PUBLIC client core)
target_include_directories(gui PUBLIC include ${CMAKE_BINARY_DIR})
