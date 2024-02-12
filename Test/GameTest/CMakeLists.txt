add_executable(GameTest GameTest.cpp)
target_link_libraries(GameTest PRIVATE Client Core Catch2::Catch2WithMain)
set_target_properties(GameTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME GameTest COMMAND GameTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

include(Packaging)
WinDeployQt(GameTest)
CopyTestDir(GameTest GameDirectories)

include(CompilerFlags)
SetCompilerFlags(GameTest)
