set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(GameFileUnpack STATIC src/GameFileUnpack.cpp)
set_target_properties(GameFileUnpack PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_include_directories(GameFileUnpack PUBLIC include)
target_link_libraries(GameFileUnpack PRIVATE Core)
include(CompilerFlags)
SetCompilerFlags(GameFileUnpack)
