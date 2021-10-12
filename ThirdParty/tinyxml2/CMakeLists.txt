set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_library(tinyxml2 STATIC "src/tinyxml2.cpp")
target_include_directories(tinyxml2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
