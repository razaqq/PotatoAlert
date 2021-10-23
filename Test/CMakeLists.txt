cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC)
    add_compile_options(/W4 /WX /EHsc)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror -fno-exceptions)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/catch2)

# enable CTest testing
enable_testing()

add_executable(PotatoTest PotatoTest.cpp GameTest.cpp ReplayTest.cpp UtilTest.cpp )

target_link_libraries(PotatoTest util client replayparser Catch2::Test)
target_include_directories(
        PotatoTest
        PUBLIC
        ${PROJECT_SOURCE_DIR}/Client
        ${PROJECT_SOURCE_DIR}/ReplayParser
        ${PROJECT_SOURCE_DIR}/Util
)
message(${PROJECT_SOURCE_DIR}/Util)

add_test(test_all PotatoTest)

file(COPY gameDirectories DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY replays DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(packaging)
windeployqt(PotatoTest)
