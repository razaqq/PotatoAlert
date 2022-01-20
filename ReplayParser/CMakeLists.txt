set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        replayparser_src

        Analyzer.cpp
        Entity.cpp
        GameFiles.cpp
        PacketParser.cpp
        ReplayParser.cpp
        Types.cpp
)

add_library(replayparser STATIC ${replayparser_src})
target_include_directories(replayparser PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(replayparser PRIVATE core)
