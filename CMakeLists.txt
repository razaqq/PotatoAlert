cmake_minimum_required(VERSION 3.16)
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

# windeployqt
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

add_custom_command(TARGET PotatoAlert POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}" --no-translations
        "$<TARGET_FILE:PotatoAlert>"
        COMMENT "Running windeployqt..."
        )

# OpenSSL DLLS
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SSL_DLLS
            "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libssl-1_1-x64.dll"
            "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libcrypto-1_1-x64.dll"
            )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SSL_DLLS
            "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libssl-1_1.dll"
            "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libcrypto-1_1.dll"
            )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif()

add_custom_command(TARGET PotatoAlert POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${SSL_DLLS}
        $<TARGET_FILE_DIR:PotatoAlert>
        COMMENT "Copying OpenSSL dlls..."
        )
