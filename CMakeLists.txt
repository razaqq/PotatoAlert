cmake_minimum_required(VERSION 3.16)
project(PotatoAlert)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets Core Gui WebSockets REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/include)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/nlohmann_json/include)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/fmt/include)

link_directories(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/lib)
link_directories(${PROJECT_SOURCE_DIR}/ThirdParty/fmt/lib)

include_directories(utils)
include_directories(gui)

add_subdirectory(utils)
add_subdirectory(gui)
add_subdirectory(test)

add_executable(PotatoAlert main.cpp Resources/PotatoAlert.rc Resources/PotatoAlert.qrc)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET PotatoAlert PROPERTY WIN32_EXECUTABLE true)
endif()

target_link_libraries(PotatoAlert
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        utilslib
        guilib
        )
target_link_libraries(PotatoAlert
        debug fmtd optimized fmt
        debug tinyxml2d optimized tinyxml2
        )

# windeployqt
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

add_custom_command(TARGET PotatoAlert POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
        "$<TARGET_FILE:PotatoAlert>"
        COMMENT "Running windeployqt..."
        )