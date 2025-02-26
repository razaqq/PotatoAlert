cmake_minimum_required(VERSION 3.17)
project(PotatoAlert VERSION 3.1.25)

option(PA_BUILD_INSTALLER "Build installer" ON)
if(WIN32)
    set(PA_BUILD_UPDATER ON)
else()
    set(PA_BUILD_INSTALLER OFF)
    set(PA_BUILD_UPDATER OFF)
endif()
message(STATUS "PotatoAlert: PA_BUILD_INSTALLER - ${PA_BUILD_INSTALLER}")
message(STATUS "PotatoAlert: PA_BUILD_UPDATER - ${PA_BUILD_UPDATER}")

find_package(Qt6 COMPONENTS Widgets REQUIRED)

if(NOT DEFINED PA_LOOKUP_URL)
    message(FATAL_ERROR "PotatoAlert: Missing cmake variable 'PA_LOOKUP_URL'")
endif()
if(NOT DEFINED PA_SUBMIT_URL)
    message(FATAL_ERROR "PotatoAlert: Missing cmake variable 'PA_SUBMIT_URL'")
endif()

pa_add_executable(
    PotatoAlert
    SOURCES
        src/PotatoAlert.cpp
        ${CMAKE_BINARY_DIR}/PotatoAlert.qrc
    PROPERTIES
        AUTORCC TRUE
        AUTOMOC TRUE
        AUTOUIC TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS TRUE
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        WIN32_EXECUTABLE $<IF:$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>,TRUE,FALSE>
    SOURCE_DEPENDENCIES
        PotatoAlert::Client
        PotatoAlert::Core
        PotatoAlert::Gui
        Qt::Widgets
    MANUAL_DEPENDENCIES
        PotatoAlert-Resources
    SOURCE_DEFINITIONS
        PA_LOOKUP_URL="${PA_LOOKUP_URL}"
        PA_SUBMIT_URL="${PA_SUBMIT_URL}"
)
pa_configure(
    PotatoAlert
    PLATFORM Windows
    SOURCE_DEPENDENCIES
        Win32
)
pa_add_version_rc(PotatoAlert
    NAME "${PROJECT_NAME}"
    FILE_DESCRIPTION "PotatoAlert"
    COMPANY_NAME "github.com/razaqq"
    ICON "${CMAKE_SOURCE_DIR}/Resources/potato.ico"
    VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR}
    VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR}
    VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH}
    VERSION_REVISION ${CMAKE_PROJECT_VERSION_TWEAK}
)
pa_install_target(
    PotatoAlert
    COMPONENT ${PROJECT_NAME}
    RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(WIN32)
    pa_windeployqt(PotatoAlert)
endif()
pa_install_qt(PotatoAlert ${PROJECT_NAME})

if(PA_BUILD_UPDATER)
    pa_add_executable(
        PotatoUpdater
        SOURCES
            src/PotatoUpdater.win32.cpp
            ${CMAKE_SOURCE_DIR}/Resources/PotatoUpdater.qrc
        PROPERTIES
            AUTORCC TRUE
            AUTOMOC TRUE
            AUTOUIC TRUE
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED TRUE
            CXX_EXTENSIONS TRUE
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            WIN32_EXECUTABLE $<IF:$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>,TRUE,FALSE>
        SOURCE_DEPENDENCIES
            PotatoAlert::Client
            PotatoAlert::Core
            PotatoAlert::Gui
            Qt::Core
            Qt::Widgets
            Win32
        MANUAL_DEPENDENCIES
            PotatoAlert-Resources
    )
    pa_add_version_rc(PotatoUpdater
        NAME "PotatoUpdater"
        FILE_DESCRIPTION "PotatoUpdater"
        COMPANY_NAME "github.com/razaqq"
        ICON "${CMAKE_SOURCE_DIR}/Resources/potato.ico"
        VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR}
        VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR}
        VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH}
        VERSION_REVISION ${CMAKE_PROJECT_VERSION_TWEAK}
    )
    pa_install_target(
        PotatoUpdater
        COMPONENT ${PROJECT_NAME}
        RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
    add_custom_command(
        TARGET PotatoUpdater
        POST_BUILD
        COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/src/PotatoUpdater.manifest\" -outputresource:$<TARGET_FILE_DIR:PotatoUpdater>/PotatoUpdater.exe\;\#1
        COMMENT "Adding manifest..."
    )
    pa_windeployqt(PotatoUpdater)
    pa_install_qt(PotatoUpdater ${PROJECT_NAME})

    if(PA_BUILD_INSTALLER)
        pa_create_installer()
    endif()
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_INSTALL_CMAKE_PROJECTS "")
set(CPACK_INSTALLED_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/" .)
set(CPACK_ARCHIVE_FILE_NAME "PotatoAlert")
set(CPACK_PACKAGE_FILE_NAME "PotatoAlert")
set(CPACK_COMPONENTS_ALL "")
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig-PotatoAlert.cmake")
set(CPACK_OUTPUT_FILE_PREFIX "")
include(CPack)
