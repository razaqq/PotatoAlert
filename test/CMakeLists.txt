cmake_minimum_required(VERSION 3.17)

project(catch2_unit_test)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty/catch2)

# enable CTest testing
enable_testing()

add_executable(PotatoTest GameTest.cpp VersionTest.cpp)

target_link_libraries(PotatoTest
        util
        Catch2::Test
        )

add_test(test_all PotatoTest)

file(COPY gameDirectories DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(packaging)
windeployqt(PotatoTest)
