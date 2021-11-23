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
set_target_properties(replayparser PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
target_link_libraries(replayparser PRIVATE util)
