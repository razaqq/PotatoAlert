cmake_minimum_required(VERSION 3.17)
project(PotatoAlert)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)  # new option() policy
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

enable_testing()

option(PA_PROFILE "Enable Profiling Output" OFF)
if(PA_PROFILE)
    add_definitions(-DPA_PROFILE=1)
endif()

if(WIN32)
    add_definitions(-D_HAS_EXCEPTIONS=0)  # turn off exceptions for ms stl
endif()
add_definitions(-DQT_MESSAGELOGCONTEXT)  # qt log messages context

include(generate_product_version)
generate_product_version(
        VersionFilesOutputVariable
        NAME "${PROJECT_NAME}"
        FILE_DESCRIPTION "PotatoAlert"
        COMPANY_NAME "github.com/razaqq"
        ICON ${PROJECT_SOURCE_DIR}/Resources/potato.ico
        VERSION_MAJOR 3
        VERSION_MINOR 0
        VERSION_PATCH 1
        VERSION_REVISION ${BUILD_REVISION}
)

# generate stringtable headers
find_package(Python3 3.7 COMPONENTS Interpreter REQUIRED)
message("Generating stringtable...")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Resources/generate_translations.py"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GENERATE_STRINGTABLE_RESULT)
if(NOT GENERATE_STRINGTABLE_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to generate stringtable")
endif()
message("Generating resource qrc...")
message("${CMAKE_CURRENT_SOURCE_DIR}/Resources")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Resources/generate_qrc.py"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
                RESULT_VARIABLE GENERATE_QRC_RESULT)
if(NOT GENERATE_QRC_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to generate resource qrc")
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/Client)
add_subdirectory(${PROJECT_SOURCE_DIR}/GameFileUnpack)
add_subdirectory(${PROJECT_SOURCE_DIR}/Gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/PotatoAlert)
add_subdirectory(${PROJECT_SOURCE_DIR}/ReplayParser)
add_subdirectory(${PROJECT_SOURCE_DIR}/Test)
add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty)
if(WIN32)
    add_subdirectory(${PROJECT_SOURCE_DIR}/Updater)
endif()

file(COPY ${PROJECT_SOURCE_DIR}/Resources/ReplayVersions DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
