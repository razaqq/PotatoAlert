cmake_minimum_required(VERSION 3.17)
project(PotatoAlert)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

cmake_policy(SET CMP0069 NEW)  # needed for CMAKE_INTERPROCEDURAL_OPTIMIZATION
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

option(PA_ENABLE_LTO "Enable cross language linking time optimization" OFF)
set(LTO_ENABLED false)
if(PA_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)

    if(LTO_SUPPORTED)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(LTO_ENABLED true)
            message(STATUS "IPO / LTO enabled")
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=thin -fuse-ld=ldd")
        else()
            message(WARNING "IPO / LTO only supported with clang")
        endif()
    else()
      message(WARNING "IPO / LTO not supported: '${LTO_ERROR}'")
    endif()
endif()

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
        VERSION_MINOR 1
        VERSION_PATCH 2
        VERSION_REVISION ${BUILD_REVISION}
)

# generate stringtable headers
find_package(Python3 3.7 COMPONENTS Interpreter REQUIRED)
message(STATUS "Generating stringtable...")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Resources/generate_translations.py"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GENERATE_STRINGTABLE_RESULT)
if(NOT GENERATE_STRINGTABLE_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to generate stringtable")
endif()
message(STATUS "Generating resource qrc...")
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
