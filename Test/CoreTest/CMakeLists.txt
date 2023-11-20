add_executable(CoreTest CoreTest.cpp)
target_link_libraries(CoreTest PRIVATE Core Catch2::Catch2WithMain)
set_target_properties(CoreTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
)

add_test(NAME CoreTest COMMAND CoreTest WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test")

include(Packaging)
WinDeployQt(CoreTest)
