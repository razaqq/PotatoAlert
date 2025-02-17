cmake_minimum_required(VERSION 3.17)
project(GameFileUnpack)

option(GameFileUnpack_STATIC "Build as static library" OFF)
option(GameFileUnpack_TEST "Build GameFileUnpack test" ON)

if(GameFileUnpack_STATIC)
    add_library(PotatoAlert-GameFileUnpack STATIC src/GameFileUnpack.cpp)
else()
    add_library(PotatoAlert-GameFileUnpack SHARED src/GameFileUnpack.cpp)
endif()
add_library(PotatoAlert::GameFileUnpack ALIAS PotatoAlert-GameFileUnpack)
set_target_properties(PotatoAlert-GameFileUnpack
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        OUTPUT_NAME "${PROJECT_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        DEBUG_POSTFIX "d"
        PUBLIC_HEADER "include/GameFileUnpack/GameFileUnpack.hpp"
)
target_include_directories(PotatoAlert-GameFileUnpack
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(PotatoAlert-GameFileUnpack PRIVATE PotatoAlert::Core)
include(CompilerFlags)
SetCompilerFlags(PotatoAlert-GameFileUnpack)

install(TARGETS PotatoAlert-GameFileUnpack
    EXPORT "${PROJECT_NAME}Targets"
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT GameFileUnpack
)
