cmake_minimum_required(VERSION 3.17)
project(PotatoAlert VERSION 3.1.25)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(PotatoAlert)

cmake_policy(SET CMP0069 NEW)  # needed for CMAKE_INTERPROCEDURAL_OPTIMIZATION
cmake_policy(SET CMP0163 NEW)  # needed for global GENERATED properties on source files
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0163 NEW)

option(PA_LTO "Enable cross language linking time optimization" ON)
message(STATUS "PotatoAlert: IPO / LTO - ${PA_LTO}")
if(PA_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_ERROR)

    if(LTO_SUPPORTED)
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(PA_LTO_ENABLED TRUE)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=thin -fuse-ld=ldd")
        else()
            message(WARNING "PotatoAlert: IPO / LTO only supported with clang")
        endif()
    else()
      message(WARNING "PotatoAlert: IPO / LTO not supported: '${LTO_ERROR}'")
    endif()
endif()

enable_testing()

option(PA_PROFILE "Enable Profiling Output" OFF)
message(STATUS "PotatoAlert: PA_PROFILE - ${PA_PROFILE}")
if(PA_PROFILE)
    add_definitions(-DPA_PROFILE=1)
endif()

if(WIN32)
    add_definitions(-D_HAS_EXCEPTIONS=0)  # turn off exceptions for ms stl
endif()
add_definitions(-DQT_MESSAGELOGCONTEXT)  # qt log messages context

add_subdirectory(${PROJECT_SOURCE_DIR}/Resources)
add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty)
add_subdirectory(${PROJECT_SOURCE_DIR}/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/Client)
add_subdirectory(${PROJECT_SOURCE_DIR}/GameFileUnpack)
add_subdirectory(${PROJECT_SOURCE_DIR}/Gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/ReplayParser)
add_subdirectory(${PROJECT_SOURCE_DIR}/PotatoAlert)
