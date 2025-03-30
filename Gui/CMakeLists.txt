cmake_minimum_required(VERSION 3.24)
project(Gui)

find_package(Qt6 COMPONENTS SvgWidgets Widgets REQUIRED)

pa_add_library(
    PotatoAlert::Gui
    SOURCES
        src/AboutWidget.cpp
        src/Events.cpp
        src/ExpandingWidget.cpp
        src/MainWindow.cpp
        src/NativeWindow.cpp
        src/Pagination.cpp
        src/Palette.cpp
        src/QuestionDialog.cpp
        src/ReplaySummary.cpp
        src/StatsParser.cpp
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
    
        # have to add some headers to sources for qt AUTOMOC
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
    PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE
        AUTOUIC TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS TRUE
        DEBUG_POSTFIX "d"
    HEADER_INCLUDE_DIRECTORIES
        include
    HEADER_DEPENDENCIES
        PotatoAlert::Client
        PotatoAlert::Core
        Qt::Widgets
    SOURCE_DEPENDENCIES
        Qt::SvgWidgets
        Qt::WidgetsPrivate
        QWKWidgets
)
pa_configure(
    PotatoAlert::Gui
    PLATFORM Windows
    SOURCES
        src/FramelessDialog.win32.cpp
        src/NativeWindow.win32.cpp
        src/Updater.cpp
    SOURCE_DEPENDENCIES
        Dwmapi
        Win32
)
pa_configure(
    PotatoAlert::Gui
    PLATFORM Linux
    SOURCES
        src/FramelessDialog.linux.cpp
        src/NativeWindow.linux.cpp
)
