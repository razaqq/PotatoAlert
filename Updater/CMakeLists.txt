# project(PotatoUpdater)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_HAS_EXCEPTIONS=0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Network REQUIRED)

add_executable(PotatoUpdater PotatoUpdater.cpp ${PROJECT_SOURCE_DIR}/Resources/PotatoAlert.qrc ${VersionFilesOutputVariable})

target_include_directories(
        PotatoUpdater
        PUBLIC
        ${PROJECT_SOURCE_DIR}/Gui
        ${PROJECT_SOURCE_DIR}/Util
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET PotatoUpdater PROPERTY WIN32_EXECUTABLE true)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")  # clang-cl
        target_compile_options(PotatoUpdater PRIVATE -Wall -Wno-unknown-pragmas -EHsc -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic)
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")  # clang
        target_compile_options(PotatoUpdater PRIVATE -Wall -pedantic -Werror -fno-exceptions -Wno-unknown-pragmas -Os)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(PotatoUpdater PRIVATE /W4 /WX /EHsc)  # TODO: completely untested
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

target_link_libraries(PotatoUpdater PRIVATE gui util)
target_link_libraries(PotatoUpdater PRIVATE Qt5::Network)
set_target_properties(PotatoUpdater PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)

add_custom_command(
        TARGET PotatoUpdater
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/PotatoUpdater.manifest\" -outputresource:"${CMAKE_CURRENT_BINARY_DIR}/PotatoUpdater.exe"\;\#1
        COMMENT "Adding manifest..."
)

include(packaging)
windeployqt(PotatoUpdater)
ssllibraries(PotatoUpdater)
