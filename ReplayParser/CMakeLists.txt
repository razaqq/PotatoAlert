option(PA_BUILD_STANDALONE_REPLAYPARSER OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(Analyzer)

add_library(ReplayParser STATIC)
target_sources(
    ReplayParser
    PRIVATE

    src/Analyzer.cpp
    src/Entity.cpp
    src/GameFiles.cpp
    src/PacketParser.cpp
    src/ReplayParser.cpp
    src/Types.cpp
)
set_target_properties(ReplayParser PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_include_directories(ReplayParser PUBLIC include)
target_link_libraries(ReplayParser PUBLIC tinyxml2 Core ReplayAnalyzer)

if(PA_BUILD_STANDALONE_REPLAYPARSER)
    message("Building standalone ReplayParser")
    add_executable(StandaloneReplayParser StandaloneReplayParser.cpp)
    target_link_libraries(StandaloneReplayParser PRIVATE ReplayParser Core)
    set_target_properties(StandaloneReplayParser PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    windeployqt(StandaloneReplayParser)
endif()
