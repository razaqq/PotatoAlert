set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(PotatoAlert-Gui STATIC)
add_library(PotatoAlert::Gui ALIAS PotatoAlert-Gui)

target_sources(
    PotatoAlert-Gui
    PRIVATE

    src/AboutWidget.cpp
    src/Events.cpp
    src/ExpandingWidget.cpp
    src/MainWindow.cpp
    src/NativeWindow.cpp
    src/Pagination.cpp
    src/Palette.cpp
    src/QuestionDialog.cpp
    src/ReplaySummary.cpp
    src/TitleBar.cpp
    src/VerticalMenuBar.cpp

    src/MatchHistory/MatchHistory.cpp
    src/MatchHistory/MatchHistoryFilter.cpp
    src/MatchHistory/MatchHistoryView.cpp
    src/MatchHistory/MatchHistoryModel.cpp

    src/SettingsWidget/GameInstalls.cpp
    src/SettingsWidget/HorizontalLine.cpp
    src/SettingsWidget/SettingsChoice.cpp
    src/SettingsWidget/SettingsSwitch.cpp
    src/SettingsWidget/SettingsWidget.cpp

    src/StatsWidget/StatsHeader.cpp
    src/StatsWidget/StatsTable.cpp
    src/StatsWidget/StatsTeamFooter.cpp
    src/StatsWidget/StatsWidget.cpp
    src/StatsWidget/TeamWidget.cpp
)

# have to add some headers to sources for qt AUTOMOC
target_sources(
    PotatoAlert-Gui
    PRIVATE

    include/Gui/MatchHistory/MatchHistory.hpp
    include/Gui/MatchHistory/MatchHistoryFilter.hpp
    include/Gui/MatchHistory/MatchHistoryView.hpp
    include/Gui/MatchHistory/MatchHistoryModel.hpp
    include/Gui/MatchHistory/ReplaySummaryButtonDelegate.hpp
    include/Gui/NativeWindow.hpp
    include/Gui/Pagination.hpp
    include/Gui/ReplaySummary.hpp
    include/Gui/SettingsWidget/GameInstalls.hpp
    include/Gui/SettingsWidget/SettingsChoice.hpp
    include/Gui/SettingsWidget/SettingsChoiceButton.hpp
    include/Gui/SettingsWidget/SettingsSlider.hpp
    include/Gui/SettingsWidget/SettingsSwitch.hpp
    include/Gui/SettingsWidget/SettingsWidget.hpp
    include/Gui/StatsWidget/StatsHeader.hpp
    include/Gui/TitleBar.hpp
    include/Gui/VerticalMenuBar.hpp
)

if(WIN32)
    target_sources(
        PotatoAlert-Gui
        PRIVATE

        src/FramelessDialog.win32.cpp
        src/NativeWindow.win32.cpp
        src/Updater.cpp
    )
endif(WIN32)

if(UNIX)
    target_sources(
        PotatoAlert-Gui
        PRIVATE

        src/FramelessDialog.linux.cpp
        src/NativeWindow.linux.cpp
    )
endif()

find_package(Qt6 COMPONENTS SvgWidgets Widgets REQUIRED)

target_link_libraries(PotatoAlert-Gui PRIVATE QWKWidgets Qt::SvgWidgets Qt::WidgetsPrivate)
target_link_libraries(PotatoAlert-Gui PUBLIC Qt::Widgets)
include(CompilerFlags)
SetCompilerFlags(PotatoAlert-Gui)

if(WIN32)
    target_link_libraries(PotatoAlert-Gui PRIVATE Dwmapi PotatoAlert::Updater Win32)
endif()

target_link_libraries(PotatoAlert-Gui PUBLIC PotatoAlert::Client PotatoAlert::Core)
target_include_directories(PotatoAlert-Gui PUBLIC include ${CMAKE_BINARY_DIR})
