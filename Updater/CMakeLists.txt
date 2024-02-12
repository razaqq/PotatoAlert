# project(PotatoUpdater)
set(CMAKE_CXX_STANDARD 23)

add_definitions(-D_HAS_EXCEPTIONS=0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Network Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Network Widgets REQUIRED)
endif()

# have to add the header to sources for qt AUTOMOC
add_library(Updater src/Updater.cpp include/Updater/Updater.hpp)
target_link_libraries(Updater PRIVATE Gui Core Qt::Network Qt::Widgets Win32)
target_include_directories(Updater PUBLIC include)
set_target_properties(Updater PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)

add_executable(PotatoUpdater src/PotatoUpdater.cpp ${PROJECT_SOURCE_DIR}/Resources/PotatoUpdater.qrc ${VersionFilesOutputVariable})

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET PotatoUpdater PROPERTY WIN32_EXECUTABLE true)
endif()

include(CompilerFlags)
SetCompilerFlags(PotatoUpdater)

target_link_libraries(PotatoUpdater PRIVATE Core Gui Updater Qt::Core Qt::Widgets Win32)
set_target_properties(PotatoUpdater PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
set_target_properties(PotatoUpdater
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(
    TARGET PotatoUpdater
    POST_BUILD
    COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/PotatoUpdater.manifest\" -outputresource:$<TARGET_FILE_DIR:PotatoUpdater>/PotatoUpdater.exe\;\#1
    COMMENT "Adding manifest..."
)

include(Packaging)
WinDeployQt(PotatoUpdater)
if (NOT Qt6_FOUND)  # only need ssl libs on qt5, not qt6
    CopySslDlls(PotatoUpdater)
endif()
