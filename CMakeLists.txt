cmake_minimum_required(VERSION 3.17)
project(PotatoAlert)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/EHa")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(generate_product_version)
generate_product_version(
        VersionFilesOutputVariable
        NAME ${PROJECT_NAME}
        FILE_DESCRIPTION "Stats tool for World of Warships"
        COMPANY_NAME github.com/razaqq
        ICON Resources/potato.ico
        VERSION_MAJOR 3
        VERSION_MINOR 0
        VERSION_PATCH ${BUILD_COUNTER}
        VERSION_REVISION ${BUILD_REVISION}
)

find_package(Qt5 COMPONENTS Widgets Core Gui WebSockets REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/include)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/nlohmann_json/include)
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty/fmt/include)

include_directories(utils)
include_directories(gui)
include_directories(Resources)

add_subdirectory(ThirdParty/tinyxml2)
add_subdirectory(ThirdParty/fmt)
add_subdirectory(utils)
add_subdirectory(gui)
add_subdirectory(test)

add_executable(PotatoAlert main.cpp Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})

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

include(packaging)
windeployqt(PotatoAlert)
ssllibraries(PotatoAlert)
# makeinstaller(PotatoAlert)
