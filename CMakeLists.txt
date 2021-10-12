cmake_minimum_required(VERSION 3.17)
project(PotatoAlert)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)  # new option() policy
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

add_definitions(-D_HAS_EXCEPTIONS=0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(generate_product_version)
generate_product_version(
        VersionFilesOutputVariable
        NAME ${PROJECT_NAME}
        FILE_DESCRIPTION "PotatoAlert"
        COMPANY_NAME github.com/razaqq
        ICON ${PROJECT_SOURCE_DIR}/Resources/potato.ico
        VERSION_MAJOR 3
        VERSION_MINOR 0
        VERSION_PATCH 1
        VERSION_REVISION ${BUILD_REVISION}
)

add_subdirectory(${PROJECT_SOURCE_DIR}/Util)
add_subdirectory(${PROJECT_SOURCE_DIR}/Client)
add_subdirectory(${PROJECT_SOURCE_DIR}/Gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/ReplayParser)
add_subdirectory(${PROJECT_SOURCE_DIR}/Test)
add_subdirectory(${PROJECT_SOURCE_DIR}/Updater)
add_subdirectory(${PROJECT_SOURCE_DIR}/FramelessHelper)

add_executable(PotatoAlert Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET PotatoAlert PROPERTY WIN32_EXECUTABLE true)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")  # clang-cl
        target_compile_options(PotatoAlert PRIVATE -Wall -Wno-unknown-pragmas -EHsc -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic)
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")  # clang
        target_compile_options(PotatoAlert PRIVATE -Wall -pedantic -Werror -fno-exceptions -Wno-unknown-pragmas -Os)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(PotatoAlert PRIVATE /W4 /WX /EHsc)  # TODO: completely untested
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

target_link_libraries(PotatoAlert PRIVATE gui)

include(packaging)
windeployqt(PotatoAlert)
ssllibraries(PotatoAlert)
# makeinstaller(PotatoAlert)
