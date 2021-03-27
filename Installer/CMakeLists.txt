set(CPACK_PACKAGE_NAME PotatoAlert)
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PRODUCT_NAME})
set(CPACK_PACKAGE_VERSION 1.0.0) # Version of installer
set(CPACK_PACKAGE_FILE_NAME "PotatoAlert Installer")
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package)
set(CPACK_GENERATOR IFW)
set(CPACK_IFW_VERBOSE ON)

include(CPack)
include(CPackIFW)

cpack_add_component(
        "PotatoAlertInstaller"
        DISPLAY_NAME ${PRODUCT_NAME}
        DESCRIPTION ${PROJECT_DESCRIPTION}
)

cpack_ifw_configure_component(
        "PotatoAlertInstaller"
        ${PRODUCT_NAME} Installer
        NAME qt.cpackifw.installer
        VERSION ${PROJECT_VERSION}
        DEFAULT TRUE
)

# add_custom_command(TARGET PotatoAlert POST_BUILD ${CPACK_IFW_BINARYCREATOR_EXECUTABLE})
