add_executable(PythonTest PythonTest.cpp)
target_link_libraries(PythonTest PRIVATE Core Catch2::Catch2WithMain)
set_target_properties(PythonTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME PythonTest COMMAND PythonTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

include(Packaging)
WinDeployQt(PythonTest)
CopyTestDir(PythonTest Pickles)
DeployPython(PythonTest)
