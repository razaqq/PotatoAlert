cmake_minimum_required(VERSION 3.17)
project(PotatoAlert)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)  # new option() policy
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

option(PA_PROFILE "Enable Profiling Output" OFF)
if(PA_PROFILE)
    add_definitions(-DPA_PROFILE=1)
endif()

add_definitions(-D_HAS_EXCEPTIONS=0)  # turn off exceptions
add_definitions(-DQT_MESSAGELOGCONTEXT)  # qt log messages context

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

add_subdirectory(${PROJECT_SOURCE_DIR}/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/Client)
add_subdirectory(${PROJECT_SOURCE_DIR}/Gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/ReplayParser)
add_subdirectory(${PROJECT_SOURCE_DIR}/Test)
add_subdirectory(${PROJECT_SOURCE_DIR}/Updater)
add_subdirectory(${PROJECT_SOURCE_DIR}/FramelessHelper)

add_executable(PotatoAlert Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})
set_target_properties(PotatoAlert PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET PotatoAlert PROPERTY WIN32_EXECUTABLE true)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")  # clang-cl
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:DEBUG>:/Wall -Wno-unknown-pragmas -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic /Ob0 /Od>)
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")  # clang
        target_compile_options(PotatoAlert PRIVATE -fno-exceptions)
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:DEBUG>:-Wall -pedantic -fno-exceptions -Wno-unknown-pragmas -O0>)
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:DEBUG>:/Wall /Od /Ob0>)  # TODO: completely untested
    target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

target_link_libraries(PotatoAlert PRIVATE gui)

include(packaging)
windeployqt(PotatoAlert)

file(COPY ${PROJECT_SOURCE_DIR}/Resources/ReplayVersions DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

ssllibraries(PotatoAlert)
# makeinstaller(PotatoAlert)
