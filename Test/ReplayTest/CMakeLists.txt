add_executable(ReplayTest ReplayTest.cpp)
target_link_libraries(ReplayTest PRIVATE Core Catch2::Catch2WithMain ReplayParser)
set_target_properties(ReplayTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME ReplayTest COMMAND ReplayTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

include(Packaging)
WinDeployQt(ReplayTest)
CopyTestDir(ReplayTest Replays)
CopyReplayScripts(ReplayTest)

include(CompilerFlags)
SetCompilerFlags(ReplayTest)
