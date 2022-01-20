cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/catch2)

# enable CTest testing
enable_testing()

add_executable(PotatoTest PotatoTest.cpp CoreTest.cpp GameTest.cpp ReplayTest.cpp)

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

set_target_properties(PotatoTest PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
target_link_libraries(PotatoTest PRIVATE core client replayparser Catch2::Test)
target_include_directories(
        PotatoTest
        PRIVATE
        ${PROJECT_SOURCE_DIR}
)

add_test(test_all PotatoTest)

file(COPY ${PROJECT_SOURCE_DIR}/Resources/ReplayVersions DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY gameDirectories DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY replays DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(packaging)
windeployqt(PotatoTest)
