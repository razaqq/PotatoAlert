set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PRODUCT_NAME})
set(CPACK_PACKAGE_VERSION 1.0.0) # Version of installer
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME} Installer")
set(CPACK_COMPONENTS_ALL qt_cpackifw_installer)
set(CPACK_GENERATOR IFW)
set(CPACK_IFW_VERBOSE ON)

include(CPack)
include(CPackIFW)

add_custom_target("deploy")

add_custom_command(
        TARGET "deploy"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/windeployqt_stuff
        COMMAND $ENV{QTDIR}/bin/windeployqt.exe --compiler-runtime --dir ${CMAKE_BINARY_DIR}/windeployqt_stuff $<TARGET_FILE:PotatoAlert>
)

install(
        DIRECTORY ${CMAKE_BINARY_DIR}/windeployqt_stuff/
        DESTINATION ${PROJECT_NAME}
        COMPONENT qt_cpackifw_installer
)

cpack_add_component(
        "${PROJECT_NAME}Installer"
        DISPLAY_NAME ${PRODUCT_NAME}
        DESCRIPTION ${PROJECT_DESCRIPTION}
)

cpack_ifw_configure_component(
        "${PROJECT_NAME}Installer"
        ${PRODUCT_NAME} Installer
        NAME qt.cpackifw.installer
        VERSION ${PROJECT_VERSION}
        DEFAULT TRUE
)

# add_custom_command(TARGET PotatoAlert POST_BUILD ${CPACK_IFW_BINARYCREATOR_EXECUTABLE})
