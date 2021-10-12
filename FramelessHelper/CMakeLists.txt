set(
        frameless_src

        FramelessUtilities.cpp
        FramelessWindowsManager.cpp
        )

if(WIN32)
    set(
        frameless_src
        ${frameless_src}

        FramelessHelper_win32.cpp
        FramelessUtilities_win32.cpp
    )
endif(WIN32)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(framelessHelper STATIC ${frameless_src})
target_link_libraries(framelessHelper PRIVATE Qt5::GuiPrivate Qt5::Widgets)

if(WIN32)
    target_link_libraries(framelessHelper PRIVATE Dwmapi)
endif(WIN32)