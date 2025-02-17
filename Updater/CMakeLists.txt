project(PotatoUpdater)

add_definitions(-D_HAS_EXCEPTIONS=0)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

# have to add the header to sources for qt AUTOMOC
add_library(PotatoAlert-Updater src/Updater.cpp include/Updater/Updater.hpp)
add_library(PotatoAlert::Updater ALIAS PotatoAlert-Updater)
target_link_libraries(PotatoAlert-Updater
    PRIVATE
        PotatoAlert::Core
        PotatoAlert::Gui
        Qt::Network
        Qt::Widgets
        Win32
)
target_include_directories(PotatoAlert-Updater
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_target_properties(PotatoAlert-Updater
    PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE
        AUTOUIC TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        DEBUG_POSTFIX "d"
)

add_executable(PotatoUpdater src/PotatoUpdater.cpp ${CMAKE_SOURCE_DIR}/Resources/PotatoUpdater.qrc ${VersionFilesOutputVariable})

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET PotatoUpdater PROPERTY WIN32_EXECUTABLE TRUE)
endif()

include(CompilerFlags)
SetCompilerFlags(PotatoUpdater)

target_link_libraries(PotatoUpdater
    PRIVATE
        PotatoAlert::Core
        PotatoAlert::Gui
        PotatoAlert::Updater
        Qt::Core
        Qt::Widgets
        Win32
)
set_target_properties(PotatoUpdater
    PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE
        AUTOUIC TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(
    TARGET PotatoUpdater
    POST_BUILD
    COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/PotatoUpdater.manifest\" -outputresource:$<TARGET_FILE_DIR:PotatoUpdater>/PotatoUpdater.exe\;\#1
    COMMENT "Adding manifest..."
)

include(Packaging)
WinDeployQt(PotatoUpdater)
