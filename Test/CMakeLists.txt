cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(PotatoTest PotatoTest.cpp CoreTest.cpp GameFileUnpackTest.cpp GameTest.cpp ReplayTest.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")  # clang-cl
        target_compile_options(PotatoTest PRIVATE $<$<CONFIG:DEBUG>:/Wall -Wno-unknown-pragmas -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic /Ob0 /Od>)
        target_compile_options(PotatoTest PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")  # clang
        target_compile_options(PotatoTest PRIVATE -fno-exceptions)
        target_compile_options(PotatoTest PRIVATE $<$<CONFIG:DEBUG>:-Wall -pedantic -fno-exceptions -Wno-unknown-pragmas -O0>)
        target_compile_options(PotatoTest PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(PotatoTest PRIVATE $<$<CONFIG:DEBUG>:/Wall /Od /Ob0>)  # TODO: completely untested
    target_compile_options(PotatoTest PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

set_target_properties(PotatoTest PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(PotatoTest PRIVATE Core Client GameFileUnpack ReplayParser Catch2::Test)
set_target_properties(PotatoTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME PotatoTest COMMAND PotatoTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

add_custom_command(TARGET PotatoTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/gameDirectories
        $<TARGET_FILE_DIR:PotatoTest>/gameDirectories
        COMMENT "Copying Game Directory Files..."
        )

add_custom_command(TARGET PotatoTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/gameFiles
        $<TARGET_FILE_DIR:PotatoTest>/gameFiles
        COMMENT "Copying Pkg Game Files..."
        )

add_custom_command(TARGET PotatoTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/replays
        $<TARGET_FILE_DIR:PotatoTest>/replays
        COMMENT "Copying Replay Test Files..."
        )

include(packaging)
replayversions(PotatoTest)
if(WIN32)
    windeployqt(PotatoTest)
endif()
