find_package(Qt5Core REQUIRED)
get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
# find_package(Python)

# find_program(BINARYCREATOR_EXECUTABLE binarycreator HINTS "${_qt_bin_dir}" ${CPACK_IFW_ROOT}/bin)
# mark_as_advanced(WINDEPLOYQT_EXECUTABLE)

# Qt5 DLLs
function(windeployqt target)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WINDEPLOYQT_ARGS --debug)
    else()
        set(WINDEPLOYQT_ARGS --release)
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/package/"
            COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            ${WINDEPLOYQT_ARGS}
            --verbose 0
            --no-compiler-runtime
            --no-angle
            --no-opengl-sw
            --no-translations
            --dir "${CMAKE_CURRENT_BINARY_DIR}/package/"
            $<TARGET_FILE:${target}>
            COMMENT "Deploying Qt..."
            )
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/package/" DESTINATION bin)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
endfunction()

# OpenSSL DLLs
function(ssllibraries target)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SSL_DLLS
                "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libssl-1_1-x64.dll"
                "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libcrypto-1_1-x64.dll"
                )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(SSL_DLLS
                "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libssl-1_1.dll"
                "${PROJECT_SOURCE_DIR}/ThirdParty/ssl/lib/libcrypto-1_1.dll"
                )
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${SSL_DLLS}
            $<TARGET_FILE_DIR:${target}>/package
            COMMENT "Copying OpenSSL dlls..."
            )
endfunction()

#[[
# Qt Installer Framework
function(makeinstaller target)
    set(CPACK_PACKAGE_VENDOR "Example_vendor")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_CONTACT "Example_vendor <example@example.com>")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")

    # set human names to execuables
    set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "Example Apps")
    set(CPACK_CREATE_DESKTOP_LINKS "${PROJECT_NAME}")
    set(CPACK_STRIP_FILES TRUE)

    #------------------------------------------------------------------------------
    # include CPack, so we get target for packages
    set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

    add_custom_target(bundle
            COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
            COMMENT "Running CPACK. Please wait..."
            DEPENDS ${PROJECT_NAME})
    set(CPACK_GENERATOR)

    # Qt IFW packaging framework
    if(BINARYCREATOR_EXECUTABLE)
        list(APPEND CPACK_GENERATOR IFW)
        message(STATUS "   + Qt Installer Framework               YES ")
    else()
        message(STATUS "   + Qt Installer Framework                NO ")
    endif()

    list(APPEND CPACK_GENERATOR ZIP)
    message(STATUS "Package generation - Windows")
    message(STATUS "   + ZIP                                  YES ")

    set(PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")

    # NSIS windows installer
    find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
    if(NSIS_PATH)
        list(APPEND CPACK_GENERATOR NSIS)
        message(STATUS "   + NSIS                                 YES ")

        set(CPACK_NSIS_DISPLAY_NAME ${CPACK_PACKAGE_NAME})
        # Icon of the installer
        file(TO_NATIVE_PATH "${PACKAGE_ICON}" CPACK_NSIS_MUI_ICON)
        file(TO_NATIVE_PATH "${PACKAGE_ICON}" CPACK_NSIS_MUI_HEADERIMAGE_BITMAP)
        set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
        set(CPACK_NSIS_MODIFY_PATH ON)
    else()
        message(STATUS "   + NSIS                                 NO ")
    endif()
endfunction()

function(makeinstaller target)
    install(
            TARGETS ${target}
            RUNTIME DESTINATION ${target}
            COMPONENT qt_cpackifw_installer
            BUNDLE DESTINATION ${target}
            COMPONENT qt_cpackifw_installer
    )

    set(CPACK_PACKAGE_NAME ${target})
    set(CPACK_PACKAGE_FILE_NAME installer)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Installation Tool")
    set(CPACK_PACKAGE_VERSION "1.0.0") # Version of installer
    set(CPACK_COMPONENTS_ALL qt_cpackifw_installer)
    set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY Qt_CPackIFW)
    set(CPACK_GENERATOR IFW)
    set(CPACK_IFW_VERBOSE ON)

    include(CPack REQUIRED)
    include(CPackIFW REQUIRED)

    cpack_add_component(
            qt_cpackifw_installer
            DISPLAY_NAME "Qt CPackIFW"
            DESCRIPTION "Install me"
            REQUIRED
    )

    cpack_ifw_configure_component(
            qt_cpackifw_installer
            FORCED_INSTALLATION
            NAME qt.cpackifw.installer
            VERSION ${PROJECT_VERSION} # Version of component
            LICENSES License ${qt_cpackifw_SOURCE_DIR}/LICENSE
            DEFAULT TRUE
    )
endfunction()
]]