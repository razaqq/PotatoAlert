find_package(Qt6 COMPONENTS Core REQUIRED)

get_target_property(_qmake_executable Qt::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# Qt DLLs
function(WinDeployQt target)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WINDEPLOYQT_BUILD_TYPE --debug)
    else()
        set(WINDEPLOYQT_BUILD_TYPE --release)
    endif()

    # --exclude-plugins only exists from 6.6.1 onwards
    if (${Qt6_VERSION} VERSION_GREATER_EQUAL "6.6.1")
        set(WINDEPLOYQT_EXCLUDES --exclude-plugins qopensslbackend)
    else()
        set(WINDEPLOYQT_EXCLUDES "")
        # TODO once 6.6.1 is usable
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
        ${WINDEPLOYQT_BUILD_TYPE}
        --verbose 0
        --no-compiler-runtime
        --no-opengl-sw
        --no-translations
        ${WINDEPLOYQT_EXCLUDES}
        --dir $<TARGET_FILE_DIR:${target}>
        $<TARGET_FILE:${target}> || (exit 0)
        COMMENT "Deploying Qt..."
    )
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/" DESTINATION bin)

    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
endfunction()
