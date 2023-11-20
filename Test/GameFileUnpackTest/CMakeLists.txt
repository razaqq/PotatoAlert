add_executable(GameFileUnpackTest GameFileUnpackTest.cpp)
target_link_libraries(GameFileUnpackTest PRIVATE Core Catch2::Catch2WithMain GameFileUnpack)
set_target_properties(GameFileUnpackTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME GameFileUnpackTest COMMAND GameFileUnpackTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

include(Packaging)
WinDeployQt(GameFileUnpackTest)
CopyTestDir(GameFileUnpackTest GameFiles)

