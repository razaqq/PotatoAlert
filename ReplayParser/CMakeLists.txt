set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        replayparser_src

        Analyzer.cpp
        Analyzer.hpp
        Entity.cpp
        Entity.hpp
        GameFiles.cpp
        GameFiles.hpp
        Packets.hpp
        PacketParser.cpp
        PacketParser.hpp
        ReplayMeta.hpp
        ReplayParser.cpp
        ReplayParser.hpp
        Types.cpp
        Types.hpp
)

add_library(replayparser STATIC ${replayparser_src})
target_include_directories(replayparser PRIVATE ${CMAKE_SOURCE_DIR}/Core)
target_link_libraries(replayparser PRIVATE core)
