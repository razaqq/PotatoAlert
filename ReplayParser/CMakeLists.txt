set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
        replayparser_src

        Packets.hpp
        Packets.cpp
        ReplayParser.cpp
        ReplayParser.hpp
        ReplayVersion.hpp
)

add_library(replayparser STATIC ${replayparser_src})
target_include_directories(replayparser PRIVATE ${CMAKE_SOURCE_DIR}/Util)
target_link_libraries(replayparser PRIVATE util)
