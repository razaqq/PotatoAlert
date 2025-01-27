add_executable(PythonTest PythonTest.cpp)
include(CompilerFlags)
SetCompilerFlags(PythonTest)
find_package(Catch2 REQUIRED)
target_link_libraries(PythonTest PRIVATE PotatoAlert::Core Catch2::Catch2WithMain)
set_target_properties(PythonTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(
    NAME PythonTest
    COMMAND PythonTest
    WORKING_DIRECTORY "${PA_TEST_DATA_DIR}"
)

DeployPython(PythonTest)
