cmake_minimum_required(VERSION 3.17)
project(PotatoAlert)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

add_definitions(-D_HAS_EXCEPTIONS=0)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options(-Wall -Wno-unknown-pragmas -EHsc -Wno-error=unused-variable -Wno-c++-compat)
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        add_compile_options(-Wall -pedantic -Werror -fno-exceptions -Wno-unknown-pragmas)
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/W4 /WX /EHsc)  # TODO: completely untested
else ()
    message(FATAL_ERROR "Unsupported compiler!")
endif ()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(generate_product_version)
generate_product_version(
        VersionFilesOutputVariable
        NAME ${PROJECT_NAME}
        FILE_DESCRIPTION "Stats Tool For World of Warships"
        COMPANY_NAME github.com/razaqq
        ICON ${PROJECT_SOURCE_DIR}/Resources/potato.ico
        VERSION_MAJOR 3
        VERSION_MINOR 0
        VERSION_PATCH 1
        VERSION_REVISION ${BUILD_REVISION}
)

find_package(Qt5 COMPONENTS Widgets Core Gui WebSockets REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/include)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/nlohmann_json/include)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/zip/src)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/win32)

include_directories(${PROJECT_SOURCE_DIR}/util)
include_directories(${PROJECT_SOURCE_DIR}/gui)
include_directories(${PROJECT_SOURCE_DIR}/Resources)
include_directories(${PROJECT_SOURCE_DIR}/framelessHelper)

add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2)
add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty/zip)

add_subdirectory(${PROJECT_SOURCE_DIR}/util)
add_subdirectory(${PROJECT_SOURCE_DIR}/gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)
add_subdirectory(${PROJECT_SOURCE_DIR}/updater)
add_subdirectory(${PROJECT_SOURCE_DIR}/framelessHelper)

add_executable(PotatoAlert main.cpp Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET PotatoAlert PROPERTY WIN32_EXECUTABLE true)
endif()

target_link_libraries(PotatoAlert
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        util
        gui
        )

include(packaging)
windeployqt(PotatoAlert)
ssllibraries(PotatoAlert)
# makeinstaller(PotatoAlert)
