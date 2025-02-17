add_executable(ReplayTest ReplayTest.cpp)
include(CompilerFlags)
SetCompilerFlags(ReplayTest)
find_package(Catch2 REQUIRED)
target_link_libraries(ReplayTest
    PRIVATE
        Catch2::Catch2WithMain
        PotatoAlert::Core
        PotatoAlert::ReplayParser
)
set_target_properties(ReplayTest
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(
    NAME ReplayTest
    COMMAND ReplayTest
    WORKING_DIRECTORY "${PA_TEST_DATA_DIR}"
)
SetDllPath(ReplayTest)
