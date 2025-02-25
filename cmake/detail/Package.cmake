find_package(Qt6 COMPONENTS Core REQUIRED)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt PATHS $ENV{QTDIR}/bin/)
if(WIN32 AND NOT WINDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "PotatoAlert: windeployqt not found")
endif()

# Qt DLLs
function(pa_windeployqt TARGET)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WINDEPLOYQT_BUILD_TYPE --debug)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(WINDEPLOYQT_BUILD_TYPE --release-with-debug-info)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(WINDEPLOYQT_BUILD_TYPE --release)
    endif()

    # --exclude-plugins only exists from 6.6.1 onwards
    if(${Qt6_VERSION} VERSION_GREATER_EQUAL "6.6.1")
        set(WINDEPLOYQT_EXCLUDES --exclude-plugins qopensslbackend)
    else()
        set(WINDEPLOYQT_EXCLUDES "")
    endif()

    add_custom_target(${TARGET}_windeployqt ALL
        ${WINDEPLOYQT_EXECUTABLE}
        ${WINDEPLOYQT_BUILD_TYPE}
        --verbose 0
        --no-compiler-runtime
        --no-opengl-sw
        --no-translations
        ${WINDEPLOYQT_EXCLUDES}
        --dir $<TARGET_FILE_DIR:${TARGET}>
        $<TARGET_FILE:${TARGET}>
        COMMENT "PotatoAlert: Deploying Qt runtime dependencies for target ${TARGET}..."
    )
endfunction()

function(pa_install_qt TARGET PROJECT_NAME)
    qt_generate_deploy_script(
        TARGET ${TARGET}
        OUTPUT_SCRIPT DEPLOY_SCRIPT
        CONTENT "
        qt_deploy_runtime_dependencies(
            EXECUTABLE $<TARGET_FILE:${TARGET}>
            BIN_DIR \"${CMAKE_INSTALL_BINDIR}\"
            LIB_DIR \"${CMAKE_INSTALL_BINDIR}\"
            PLUGINS_DIR \"${CMAKE_INSTALL_BINDIR}\"
            NO_TRANSLATIONS
            NO_COMPILER_RUNTIME
            DEPLOY_TOOL_OPTIONS --no-opengl-sw --exclude-plugins qopensslbackend --no-ffmpeg
        )"
    )
    install(SCRIPT ${DEPLOY_SCRIPT} COMPONENT ${PROJECT_NAME})
    set_property(INSTALL "$<TARGET_FILE_NAME:${TARGET}>" PROPERTY CPACK_START_MENU_SHORTCUTS ${TARGET})
endfunction()

function(pa_archive_component COMPONENT)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_ARCHIVE_FILE_NAME "${COMPONENT}")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_ALL ${COMPONENT})
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
    set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig-${COMPONENT}.cmake")
    set(CPACK_OUTPUT_FILE_PREFIX "")
    include(CPack)
endfunction()

function(pa_create_installer)
    set(CPACK_GENERATOR "WIX")
    set(CPACK_PACKAGE_NAME "PotatoAlert")
    set(CPACK_PACKAGE_VENDOR "razaqq")
    set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/razaqq/PotatoAlert")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION "A statistics companion app for World Of Warships")
    set(CPACK_WIX_UPGRADE_GUID "E3D9B068-8320-484E-9F97-0C40E264B665")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/Resources/potato.ico")
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/Resources/potato.ico")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/Resources/Installer/WixBanner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/Resources/Installer/WixDialog.png")
    set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

    configure_file("${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/LICENSE.txt" COPYONLY)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
    set(CPACK_WIX_CULTURES "en-US;de-DE;ja-JP;it-IT;pt-BR;zh-CN;zh-TW")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "PotatoAlert")
    set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN TRUE)
    set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED TRUE)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CPACK_STRIP_FILES TRUE)
    endif()
    #set(CPACK_COMPONENTS_ALL PotatoAlert)
    set(CPACK_COMPONENTS_ALL "")
    set(CPACK_INSTALL_CMAKE_PROJECTS "")
    #set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};PotatoAlert;PotatoAlert;/")
    set(CPACK_INSTALLED_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/" .)
    include(CPack)
endfunction()

function(pa_install_target TARGET)
    get_target_property(ALIASED_TARGET ${TARGET} ALIASED_TARGET)
    if(NOT "${ALIASED_TARGET}" STREQUAL "ALIASED_TARGET-NOTFOUND")
        set(TARGET ${ALIASED_TARGET})
    endif()

    cmake_parse_arguments(PA_OPT
        ""
        "COMPONENT;RUNTIME_DESTINATION;LIBRARY_DESTINATION;ARCHIVE_DESTINATION;HEADERS_DESTINATION"
        ""
        ${ARGN}
    )

    set(ARGS TARGETS ${TARGET})
    if(DEFINED PA_OPT_COMPONENT)
        list(APPEND ARGS COMPONENT ${PA_OPT_COMPONENT})
    endif()
    if(DEFINED PA_OPT_RUNTIME_DESTINATION)
        list(APPEND ARGS RUNTIME DESTINATION ${PA_OPT_RUNTIME_DESTINATION})
    endif()
    if(DEFINED PA_OPT_LIBRARY_DESTINATION)
        list(APPEND ARGS LIBRARY DESTINATION ${PA_OPT_LIBRARY_DESTINATION})
    endif()
    if(DEFINED PA_OPT_ARCHIVE_DESTINATION)
        list(APPEND ARGS ARCHIVE DESTINATION ${PA_OPT_ARCHIVE_DESTINATION})
    endif()
    if(DEFINED PA_OPT_HEADERS_DESTINATION)
        list(APPEND ARGS FILE_SET "HEADERS" DESTINATION ${PA_OPT_HEADERS_DESTINATION})
    endif()

    install(${ARGS})
endfunction()
