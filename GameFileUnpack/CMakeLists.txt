set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(PotatoAlert-GameFileUnpack STATIC src/GameFileUnpack.cpp)
set_target_properties(PotatoAlert-GameFileUnpack PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_include_directories(PotatoAlert-GameFileUnpack PUBLIC include)
target_link_libraries(PotatoAlert-GameFileUnpack PRIVATE PotatoAlert::Core)
include(CompilerFlags)
SetCompilerFlags(PotatoAlert-GameFileUnpack)
