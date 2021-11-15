set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        replayparser_src

        ByteUtil.hpp
        Entity.cpp
        Entity.hpp
        GameFiles.cpp
        GameFiles.hpp
        Packets.hpp
        Packets.cpp
        PacketParser.hpp
        ReplayMeta.hpp
        ReplayParser.cpp
        ReplayParser.hpp
        Types.cpp
        Types.hpp
)

add_library(replayparser STATIC ${replayparser_src})
set_target_properties(replayparser PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
