set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(Gui STATIC)

target_sources(
    Gui
    PRIVATE

    src/AboutWidget.cpp
    src/LanguageChangeEvent.cpp
    src/MainWindow.cpp
    src/MatchHistory.cpp
    src/NativeWindow.cpp
    src/Palette.cpp
    src/QuestionDialog.cpp
    src/ReplaySummary.cpp
    src/TitleBar.cpp
    src/VerticalMenuBar.cpp

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
    Gui
    PRIVATE

    include/Gui/MatchHistory.hpp
    include/Gui/NativeWindow.hpp
    include/Gui/ReplaySummary.hpp
    include/Gui/SettingsWidget/SettingsChoiceButton.hpp
    include/Gui/SettingsWidget/SettingsSwitch.hpp
    include/Gui/SettingsWidget/SettingsWidget.hpp
    include/Gui/StatsWidget/StatsHeader.hpp
    include/Gui/TitleBar.hpp
    include/Gui/VerticalMenuBar.hpp
)

if(WIN32)
    target_sources(
        Gui
        PRIVATE

        src/FramelessDialog.win32.cpp
        src/NativeWindow.win32.cpp
        src/Updater.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        Gui
        PRIVATE

        src/FramelessDialog.linux.cpp
        src/NativeWindow.linux.cpp
    )
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED)

target_link_libraries(Gui PRIVATE FramelessHelperWidgets Qt6::Widgets)

if(WIN32)
    target_link_libraries(Gui PRIVATE Dwmapi Win32 Updater)
endif()

target_link_libraries(Gui PUBLIC Client Core)
target_include_directories(Gui PUBLIC include ${CMAKE_BINARY_DIR})
