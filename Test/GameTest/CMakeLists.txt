add_executable(GameTest GameTest.cpp)
include(CompilerFlags)
SetCompilerFlags(GameTest)
find_package(Catch2 REQUIRED)
target_link_libraries(GameTest PRIVATE Catch2::Catch2WithMain PotatoAlert::Client PotatoAlert::Core)
set_target_properties(GameTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(
    NAME GameTest
    COMMAND GameTest
    WORKING_DIRECTORY "${PA_TEST_DATA_DIR}"
)
SetDllPath(GameTest)
