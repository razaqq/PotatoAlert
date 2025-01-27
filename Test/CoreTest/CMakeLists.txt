add_executable(CoreTest CoreTest.cpp)
include(CompilerFlags)
SetCompilerFlags(CoreTest)
find_package(Catch2 REQUIRED)
target_link_libraries(CoreTest PRIVATE Catch2::Catch2WithMain PotatoAlert::Core)
set_target_properties(CoreTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(
    NAME CoreTest
    COMMAND CoreTest
    WORKING_DIRECTORY "${PA_TEST_DATA_DIR}"
)
