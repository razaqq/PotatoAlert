# project(PotatoUpdater)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_HAS_EXCEPTIONS=0)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options(-Wall -Wno-unknown-pragmas -EHsc -Wno-error=unused-variable -Wno-c++-compat)
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        add_compile_options(-Wall -pedantic -Werror -fno-exceptions -Wno-unknown-pragmas)
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/W4 /WX /EHsc)  # TODO: completely untested
else ()
    message(FATAL_ERROR "Unsupported compiler!")
endif ()

find_package(Qt5 COMPONENTS Widgets Core Gui WebSockets REQUIRED)

add_executable(PotatoUpdater main.cpp ${PROJECT_SOURCE_DIR}/Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET PotatoUpdater PROPERTY WIN32_EXECUTABLE true)
endif()

target_link_libraries(PotatoUpdater
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        util
        zip
        gui
        Dwmapi
        )

add_custom_command(
        TARGET PotatoUpdater
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/PotatoUpdater.manifest\" -outputresource:"${CMAKE_CURRENT_BINARY_DIR}/PotatoUpdater.exe"\;\#1
        COMMENT "Adding manifest..."
)

include(packaging)
windeployqt(PotatoUpdater)
ssllibraries(PotatoUpdater)
