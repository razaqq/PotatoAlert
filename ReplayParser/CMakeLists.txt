set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(replayparser STATIC)
target_sources(
    replayparser
    PRIVATE

    src/Analyzer.cpp
    src/Entity.cpp
    src/GameFiles.cpp
    src/PacketParser.cpp
    src/ReplayParser.cpp
    src/Types.cpp
)
set_target_properties(replayparser PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_include_directories(replayparser PUBLIC include)
target_link_libraries(replayparser PUBLIC tinyxml2)
target_link_libraries(replayparser PRIVATE core)
