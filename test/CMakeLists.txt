cmake_minimum_required(VERSION 3.5)

# Set the project name
project(catch2_unit_test)


set(CMAKE_CXX_STANDARD 17)

# add the CMakeFile that defines catch2
add_subdirectory(${PROJECT_SOURCE_DIR}/ThirdParty/catch2)

# enable CTest testing
enable_testing()

# Add a testing executable
add_executable(PotatoTest Test.cpp)

target_link_libraries(PotatoTest
        utilslib
        Catch2::Test
        )

add_test(test_all PotatoTest)

add_custom_command(TARGET PotatoTest POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
        "$<TARGET_FILE:PotatoTest>"
        COMMENT "Running windeployqt..."
        )

file(COPY gameDirectories DESTINATION ${CMAKE_CURRENT_BINARY_DIR})