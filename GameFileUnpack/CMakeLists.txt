project(GameFileUnpack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(PotatoAlert-GameFileUnpack SHARED src/GameFileUnpack.cpp)
add_library(PotatoAlert::GameFileUnpack ALIAS PotatoAlert-GameFileUnpack)
set_target_properties(PotatoAlert-GameFileUnpack
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED true
        OUTPUT_NAME "${PROJECT_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_include_directories(PotatoAlert-GameFileUnpack PUBLIC include)
target_link_libraries(PotatoAlert-GameFileUnpack PRIVATE PotatoAlert::Core)
include(CompilerFlags)
SetCompilerFlags(PotatoAlert-GameFileUnpack)
