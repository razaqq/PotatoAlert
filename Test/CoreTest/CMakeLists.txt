add_executable(CoreTest CoreTest.cpp)
find_package(Catch2 REQUIRED)
target_link_libraries(CoreTest PRIVATE Core Catch2::Catch2WithMain)
set_target_properties(CoreTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME CoreTest COMMAND CoreTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

include(Packaging)
CopyTestDir(CoreTest Misc)

include(CompilerFlags)
SetCompilerFlags(CoreTest)
