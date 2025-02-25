project(MinimapRenderer)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(ffmpeg REQUIRED)
find_package(opencv REQUIRED)
find_package(Simd REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(Python 3.13 EXACT REQUIRED COMPONENTS Development.Embed)


file(
    GLOB_RECURSE
    PYTHON_RUNTIME_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/python/runtime/*.*"
    LIST_DIRECTORIES FALSE
)
string(REGEX REPLACE "python/runtime" "python" PYTHON_RUNTIME_FILES ${PYTHON_RUNTIME_FILES})
add_custom_command(
    OUTPUT ${PYTHON_RUNTIME_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/python/runtime"
        "${CMAKE_BINARY_DIR}/bin/python"
    COMMENT "Copying MinimapRenderer Python Runtime"
    )
    add_custom_target(PotatoAlert-MinimapRendererRuntime
    DEPENDS ${PYTHON_RUNTIME_FILES}
)

if(WIN32)
    set(LIB_TYPE SHARED)
elseif(LINUX)
    set(LIB_TYPE STATIC)
else()
    message(FATAL_ERROR "PotatoAlert: Unsupported OS")
endif()
message(STATUS "PotatoAlert: Building MinimapRenderer as ${LIB_TYPE} library")

pa_add_library(
    PotatoAlert::MinimapRenderer
    ${LIB_TYPE}
    HEADERS
        include/MinimapRenderer/AchievementRibbon.hpp
        include/MinimapRenderer/Avatar.hpp
        include/MinimapRenderer/BattleLogic.hpp
        include/MinimapRenderer/Building.hpp
        include/MinimapRenderer/CapturePoint.hpp
        include/MinimapRenderer/Chat.hpp
        include/MinimapRenderer/ConsumableStates.hpp
        include/MinimapRenderer/Counters.hpp
        include/MinimapRenderer/Entity.hpp
        include/MinimapRenderer/EntityManager.hpp
        include/MinimapRenderer/Hitpoints.hpp
        include/MinimapRenderer/InteractiveZone.hpp
        include/MinimapRenderer/KillFeed.hpp
        include/MinimapRenderer/MinimapRenderer.hpp
        include/MinimapRenderer/Plane.hpp
        include/MinimapRenderer/Python.hpp
        include/MinimapRenderer/ResourceManager.hpp
        include/MinimapRenderer/SmokeScreen.hpp
        include/MinimapRenderer/Util.hpp
        include/MinimapRenderer/Vehicle.hpp
        include/MinimapRenderer/VideoWriter.hpp
    SOURCES
        src/AchievementRibbon.cpp
        src/Avatar.cpp
        src/BattleLogic.cpp
        src/Building.cpp
        src/CapturePoint.cpp
        src/Chat.cpp
        src/ConsumableStates.cpp
        src/Counters.cpp
        src/Entity.cpp
        src/EntityManager.cpp
        src/Hitpoints.cpp
        src/InteractiveZone.cpp
        src/KillFeed.cpp
        src/MinimapRenderer.cpp
        src/Plane.cpp
        src/Python.cpp
        src/ResourceManager.cpp
        src/SmokeScreen.cpp
        src/Util.cpp
        src/Vehicle.cpp
        src/VideoWriter.cpp
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
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        opencv_freetype
        PotatoAlert::Core
        PotatoAlert::GameFileUnpack
        PotatoAlert::ReplayParser
    SOURCE_DEPENDENCIES
        Bcrypt
        ffmpeg::ffmpeg
        Python::Python
        Simd::Simd
        tinyxml2::tinyxml2
        Qt::Core
    MANUAL_DEPENDENCIES
        PotatoAlert-MinimapRendererRuntime
        PotatoAlert-Resources
)

install(FILES $<IF:$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>,${Python_RUNTIME_LIBRARY_RELEASE},${Python_RUNTIME_LIBRARY_DEBUG}> DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT ${PROJECT_NAME})
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python/runtime/" DESTINATION "${CMAKE_INSTALL_BINDIR}/python" COMPONENT ${PROJECT_NAME})
pa_install_target(
    PotatoAlert::MinimapRenderer
    COMPONENT ${PROJECT_NAME}
    RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
pa_archive_component(${PROJECT_NAME})
