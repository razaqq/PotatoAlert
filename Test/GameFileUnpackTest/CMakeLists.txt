add_executable(GameFileUnpackTest GameFileUnpackTest.cpp)
include(CompilerFlags)
SetCompilerFlags(GameFileUnpackTest)
find_package(Catch2 REQUIRED)
target_link_libraries(GameFileUnpackTest
    PRIVATE
        Catch2::Catch2WithMain
        PotatoAlert::Core
        PotatoAlert::GameFileUnpack
)
set_target_properties(GameFileUnpackTest
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(
    NAME GameFileUnpackTest
    COMMAND GameFileUnpackTest
    WORKING_DIRECTORY "${PA_TEST_DATA_DIR}"
)
SetDllPath(GameFileUnpackTest)
