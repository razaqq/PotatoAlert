project(ReplayParser)

option(PA_BUILD_STANDALONE_REPLAYPARSER OFF)
message(STATUS "PotatoAlert: PA_BUILD_STANDALONE_REPLAYPARSER - ${PA_BUILD_STANDALONE_REPLAYPARSER}")

find_package(tinyxml2 REQUIRED)
add_subdirectory(Analyzer)

if(WIN32)
    set(LIB_TYPE SHARED)
elseif(LINUX)
    set(LIB_TYPE STATIC)
else()
    message(FATAL_ERROR "PotatoAlert: Unsupported OS")
endif()
message(STATUS "PotatoAlert: Building ReplayParser as ${LIB_TYPE} library")

pa_add_library(
    PotatoAlert::ReplayParser
    ${LIB_TYPE}
    HEADERS
        include/ReplayParser/BitReader.hpp
        include/ReplayParser/Entity.hpp
        include/ReplayParser/NestedProperty.hpp
        include/ReplayParser/PacketCallback.hpp
        include/ReplayParser/PacketParser.hpp
        include/ReplayParser/Packets.hpp
        include/ReplayParser/ReplayMeta.hpp
        include/ReplayParser/ReplayParser.hpp
        include/ReplayParser/Result.hpp
        include/ReplayParser/Types.hpp
        include/ReplayParser/Variant.hpp
    SOURCES
        src/Analyzer.cpp
        src/BitReader.cpp
        src/Entity.cpp
        src/GameFiles.cpp
        src/NestedProperty.cpp
        src/Packets.cpp
        src/ReplayParser.cpp
        src/Types.cpp
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS TRUE
        OUTPUT_NAME "${PROJECT_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        DEBUG_POSTFIX "d"
    HEADER_INCLUDE_DIRECTORIES
        include
    HEADER_DEPENDENCIES
        tinyxml2::tinyxml2
        PotatoAlert::Core
        PotatoAlert::ReplayAnalyzer
    TEST_SOURCES
        Test/ReplayParserTest.cpp
    TEST_PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-test"
    TEST_WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/Data"
)
pa_install_target(
    PotatoAlert::ReplayParser
    COMPONENT ${PROJECT_NAME}
    RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
pa_archive_component(${PROJECT_NAME})

if(PA_BUILD_STANDALONE_REPLAYPARSER)
    pa_add_executable(StandaloneReplayParser
        SOURCES
            StandaloneReplayParser.cpp
        PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED TRUE
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        SOURCE_DEPENDENCIES
            PotatoAlert::Core
            PotatoAlert::ReplayParser
    )
endif()
