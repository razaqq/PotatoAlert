set(
        frameless_src

        FramelessHelper_win32.cpp
        FramelessUtilities.cpp
        FramelessUtilities_win32.cpp
        FramelessWindowsManager.cpp
        )

add_library(framelessHelper STATIC ${frameless_src})
target_link_libraries(framelessHelper PRIVATE dwmapi Qt5::GuiPrivate Qt5::Widgets)
