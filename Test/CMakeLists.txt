cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(CoreTest)
add_subdirectory(Data)
add_subdirectory(GameFileUnpackTest)
add_subdirectory(GameTest)
add_subdirectory(ReplayTest)
