add_library(framelessHelper STATIC)

target_sources(
        framelessHelper
        PRIVATE

        src/FramelessUtilities.cpp
        src/FramelessWindowsManager.cpp
)

if(WIN32)
    target_sources(
            framelessHelper
            PRIVATE

            src/FramelessHelper_win32.cpp
            src/FramelessUtilities_win32.cpp
    )
endif(WIN32)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

target_link_libraries(framelessHelper PRIVATE Qt5::GuiPrivate Qt5::Widgets)
target_include_directories(framelessHelper PUBLIC include)

if(WIN32)
    target_link_libraries(framelessHelper PRIVATE Dwmapi)
endif(WIN32)