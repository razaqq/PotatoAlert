# project(PotatoUpdater)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_HAS_EXCEPTIONS=0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

# have to add the header to sources for qt AUTOMOC
add_library(Updater src/Updater.cpp include/Updater/Updater.hpp)
target_link_libraries(Updater PRIVATE Gui Core Qt6::Network Qt6::Widgets Win32)
target_include_directories(Updater PUBLIC include)
set_target_properties(Updater PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)

add_executable(PotatoUpdater src/PotatoUpdater.cpp ${PROJECT_SOURCE_DIR}/Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET PotatoUpdater PROPERTY WIN32_EXECUTABLE true)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")  # clang-cl
        target_compile_options(PotatoUpdater PRIVATE $<$<CONFIG:DEBUG>:/Wall -Wno-unknown-pragmas -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic /Ob0 /Od>)
        target_compile_options(PotatoUpdater PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")  # clang
        target_compile_options(PotatoUpdater PRIVATE -fno-exceptions)
        target_compile_options(PotatoUpdater PRIVATE $<$<CONFIG:DEBUG>:-Wall -pedantic -fno-exceptions -Wno-unknown-pragmas -O0>)
        target_compile_options(PotatoUpdater PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(PotatoUpdater PRIVATE $<$<CONFIG:DEBUG>:/Wall /Od /Ob0>)  # TODO: completely untested
    target_compile_options(PotatoUpdater PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

target_link_libraries(PotatoUpdater PRIVATE Core Gui Updater Qt6::Core Qt6::Widgets Win32)
set_target_properties(PotatoUpdater PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
set_target_properties(PotatoUpdater
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(
        TARGET PotatoUpdater
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/PotatoUpdater.manifest\" -outputresource:$<TARGET_FILE_DIR:PotatoUpdater>/PotatoUpdater.exe\;\#1
        COMMENT "Adding manifest..."
)

include(packaging)
windeployqt(PotatoUpdater)
ssllibraries(PotatoUpdater)
