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
set_target_properties(replayparser PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
target_include_directories(replayparser PRIVATE ${CMAKE_SOURCE_DIR}/Util)
target_link_libraries(replayparser PRIVATE util)
