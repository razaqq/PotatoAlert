set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

add_executable(PotatoAlert ${CMAKE_BINARY_DIR}/PotatoAlert.qrc ${VersionFilesOutputVariable})
set_target_properties(PotatoAlert PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)

if(WIN32)
    target_sources(PotatoAlert PRIVATE PotatoAlert.win32.cpp)
    target_link_libraries(PotatoAlert PRIVATE Updater Win32)
endif()

if(UNIX)
    target_sources(PotatoAlert PRIVATE PotatoAlert.linux.cpp)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET PotatoAlert PROPERTY WIN32_EXECUTABLE true)
endif()

include(CompilerFlags)
SetCompilerFlags(PotatoAlert)

target_link_libraries(PotatoAlert PRIVATE Core Gui Qt::Widgets)

set_target_properties(PotatoAlert
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include(Packaging)
if(WIN32)
    WinDeployQt(PotatoAlert)
    if (NOT Qt6_FOUND)  # only need ssl libs on qt5, not qt6
        CopySslDlls(PotatoAlert)
    endif()
endif()
# makeinstaller(PotatoAlert)

install(DIRECTORY "${CMAKE_BINARY_DIR}/bin/" DESTINATION . COMPONENT PotatoAlert)
set_property(INSTALL "$<TARGET_FILE_NAME:PotatoAlert>" PROPERTY CPACK_START_MENU_SHORTCUTS "PotatoAlert")
