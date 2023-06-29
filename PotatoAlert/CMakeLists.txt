set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

add_executable(PotatoAlert ${PROJECT_SOURCE_DIR}/Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})
set_target_properties(PotatoAlert PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)

if(WIN32)
    target_sources(PotatoAlert PRIVATE PotatoAlert.win32.cpp)
    target_link_libraries(PotatoAlert PRIVATE Updater Win32)
endif()

if(UNIX)
    target_sources(PotatoAlert PRIVATE PotatoAlert.linux.cpp)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET PotatoAlert PROPERTY WIN32_EXECUTABLE true)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")  # clang-cl
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:DEBUG>:/Wall -Wno-unknown-pragmas -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic /Ob0 /Od>)
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")  # clang
        target_compile_options(PotatoAlert PRIVATE -fno-exceptions)
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:DEBUG>:-Wall -pedantic -Wno-unknown-pragmas -O0>)
        target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:DEBUG>:/Wall /Od /Ob0>)  # TODO: completely untested
    target_compile_options(PotatoAlert PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

target_link_libraries(PotatoAlert PRIVATE Core Gui Qt::Widgets FramelessHelper::Widgets)

set_target_properties(PotatoAlert
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include(packaging)
if(WIN32)
    windeployqt(PotatoAlert)
    if (NOT Qt6_FOUND)  # only need ssl libs on qt5, not qt6
        ssllibraries(PotatoAlert)
    endif()
endif()
# makeinstaller(PotatoAlert)
