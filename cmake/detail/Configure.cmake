set(PA_DETAIL_CONFIGURE_OPT_1
    FOLDER
    TEST_WORKING_DIRECTORY
)
set(PA_DETAIL_CONFIGURE_OPT_N
    SOURCES
    SOURCE_DEPENDENCIES
    SOURCE_DEFINITIONS
    SOURCE_INCLUDE_DIRECTORIES
    MANUAL_DEPENDENCIES
    PROPERTIES
    VISUALIZERS
)
set(PA_DETAIL_CONFIGURE_LIB_OPT_0
    STATIC
    SHARED
    MODULE
    TEST_COMPILE_ONLY
)
set(PA_DETAIL_CONFIGURE_LIB_OPT_N
    HEADERS
    HEADER_DEPENDENCIES
    HEADER_DEFINITIONS
    HEADER_INCLUDE_DIRECTORIES
    TEST_SOURCES
    TEST_DEPENDENCIES
    TEST_PROPERTIES
)

macro(pa_detail_configure_parse TYPE OPT_0 OPT_1 OPT_N)
    set(PA_DETAIL_OPT_0 ${OPT_0})
    set(PA_DETAIL_OPT_1 ${OPT_1})
    set(PA_DETAIL_OPT_N ${OPT_N})

    list(APPEND PA_DETAIL_OPT_1 ${PA_DETAIL_CONFIGURE_OPT_1})
    list(APPEND PA_DETAIL_OPT_N ${PA_DETAIL_CONFIGURE_OPT_N})

    if(DEFINED PA_DETAIL_CONFIGURE_${TYPE}_OPT_0)
        list(APPEND PA_DETAIL_OPT_0 ${PA_DETAIL_CONFIGURE_${TYPE}_OPT_0})
    endif()
    if(DEFINED PA_DETAIL_CONFIGURE_${TYPE}_OPT_1)
        list(APPEND PA_DETAIL_OPT_1 ${PA_DETAIL_CONFIGURE_${TYPE}_OPT_1})
    endif()
    if(DEFINED PA_DETAIL_CONFIGURE_${TYPE}_OPT_N)
        list(APPEND PA_DETAIL_OPT_N ${PA_DETAIL_CONFIGURE_${TYPE}_OPT_N})
    endif()

    cmake_parse_arguments(
        PA_OPT
        "${PA_DETAIL_OPT_0}"
        "${PA_DETAIL_OPT_1}"
        "${PA_DETAIL_OPT_N}"
        ${ARGN}
    )

    if(DEFINED PA_OPT_UNPARSED_ARGUMENTS)
        message(SEND_ERROR "pa_configure: unrecognized arguments: ${PA_OPT_UNPARSED_ARGUMENTS}")
    endif()
endmacro()

function(pa_detail_add_target FUNCTION NAME)
    string(REPLACE "::" "_" TARGET ${NAME})
    string(REPLACE "+" "_" TARGET ${TARGET})
    string(REPLACE "-" "_" TARGET ${TARGET})
    string(REPLACE "." "_" TARGET ${TARGET})

    cmake_language(CALL ${FUNCTION} ${TARGET} ${ARGN})

    if(NOT "x${TARGET}" STREQUAL "x${NAME}")
        cmake_language(CALL ${FUNCTION} ${NAME} ALIAS ${TARGET})
    endif()

    set(TARGET ${TARGET} PARENT_SCOPE)
    set(DO_CONFIGURE ON PARENT_SCOPE)
endfunction()

function(pa_detail_configure NAME TARGET PUBLIC_TYPE)
    if(DEFINED PA_OPT_HEADERS)
        target_sources(${TARGET}
            PUBLIC FILE_SET "HEADERS" BASE_DIRS "include" FILES "${PA_OPT_HEADERS}"
        )
    endif()

    if(DEFINED PA_OPT_SOURCES)
        if(DO_CONFIGURE)
            target_sources(${TARGET} PRIVATE ${PA_OPT_SOURCES})
        endif()
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_HEADER_DEPENDENCIES)
        target_link_libraries(${TARGET} ${PUBLIC_TYPE} ${PA_OPT_HEADER_DEPENDENCIES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_SOURCE_DEPENDENCIES)
        target_link_libraries(${TARGET} PRIVATE ${PA_OPT_SOURCE_DEPENDENCIES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_HEADER_DEFINITIONS)
        target_compile_definitions(${TARGET} ${PUBLIC_TYPE} ${PA_OPT_HEADER_DEFINITIONS})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_SOURCE_DEFINITIONS)
        target_compile_definitions(${TARGET} PRIVATE ${PA_OPT_SOURCE_DEFINITIONS})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_PROPERTIES)
        set_target_properties(${TARGET} PROPERTIES ${PA_OPT_PROPERTIES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_FOLDER)
        set_target_properties(${TARGET} PROPERTIES FOLDER "${PA_OPT_FOLDER}")
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_SOURCE_INCLUDE_DIRECTORIES)
        target_include_directories(${TARGET} PRIVATE ${PA_OPT_SOURCE_INCLUDE_DIRECTORIES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_HEADER_INCLUDE_DIRECTORIES)
        target_include_directories(${TARGET} ${PUBLIC_TYPE} ${PA_OPT_HEADER_INCLUDE_DIRECTORIES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_MANUAL_DEPENDENCIES)
        set(MANUAL_DEPENDENCIES "")
        foreach(DEPENDENCY IN LISTS PA_OPT_MANUAL_DEPENDENCIES)
            get_target_property(DEPENDENCY_ALIASED_TARGET ${DEPENDENCY} ALIASED_TARGET)
            if(NOT "${DEPENDENCY_ALIASED_TARGET}" STREQUAL "DEPENDENCY_ALIASED_TARGET-NOTFOUND")
                list(APPEND MANUAL_DEPENDENCIES ${DEPENDENCY_ALIASED_TARGET})
            else()
                list(APPEND MANUAL_DEPENDENCIES ${DEPENDENCY})
            endif()
        endforeach()
        add_dependencies(${TARGET} ${MANUAL_DEPENDENCIES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_TEST_SOURCES)
        if(NOT "${PA_OPT_TEST_COMPILE_ONLY}" AND NOT "${Catch2_FOUND}")
            find_package(Catch2 REQUIRED QUIET)
        endif()

        get_target_property(TEST_TARGET ${TARGET} PA_DETAIL_TEST_TARGET)

        if(${TEST_TARGET} STREQUAL "TEST_TARGET-NOTFOUND")
            set(TEST_TARGET ${TARGET}Test)
            set_property(TARGET ${TARGET} PROPERTY PA_DETAIL_TEST_TARGET ${TEST_TARGET})

            if("${PA_OPT_TEST_COMPILE_ONLY}")
                add_library(${TEST_TARGET})
            else()
                add_executable(${TEST_TARGET})
                if(DEFINED PA_OPT_TEST_WORKING_DIRECTORY)
                    add_test(
                        NAME ${TEST_TARGET}
                        COMMAND ${TEST_TARGET}
                        WORKING_DIRECTORY "${PA_OPT_TEST_WORKING_DIRECTORY}"
                    )
                else()
                    add_test(
                        NAME ${TEST_TARGET}
                        COMMAND ${TEST_TARGET}
                    )
                endif()

                get_target_property(TARGET_RUNTIME_OUTPUT_DIRECTORY ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
                if(NOT "${TARGET_RUNTIME_OUTPUT_DIRECTORY}" STREQUAL "TARGET_RUNTIME_OUTPUT_DIRECTORY-NOTFOUND" AND "${PA_OPT_SHARED}")
                    string(REPLACE "\\" "/" ESCAPED_PATH "$ENV{PATH}")
                    set_tests_properties(${TEST_TARGET}
                        PROPERTIES
                            ENVIRONMENT "PATH=$<JOIN:${TARGET_RUNTIME_OUTPUT_DIRECTORY};${ESCAPED_PATH},\\;>")
                endif()
            endif()

            target_link_libraries(${TEST_TARGET} PRIVATE ${TARGET} pa_detail_cxx_options)

            if(NOT "${PA_OPT_TEST_COMPILE_ONLY}")
                target_link_libraries(${TEST_TARGET} PRIVATE ${TARGET} Catch2::Catch2WithMain)
            endif()

            if (DEFINED PA_OPT_TEST_PROPERTIES)
                set_target_properties(${TEST_TARGET} PROPERTIES ${PA_OPT_TEST_PROPERTIES})
            endif()
        endif()

        target_sources(${TEST_TARGET} PRIVATE ${PA_OPT_TEST_SOURCES})
    endif()

    if(DO_CONFIGURE AND DEFINED PA_OPT_TEST_DEPENDENCIES)
        get_target_property(TEST_TARGET ${target} PA_DETAIL_TEST_TARGET)

        if(NOT ${TEST_TARGET} STREQUAL "test_target-NOTFOUND")
            target_link_libraries(${TEST_TARGET} PRIVATE ${PA_OPT_TEST_DEPENDENCIES})
        else()
            message(SEND_ERROR "Cannot specify test dependencies for target ${TARGET} without test sources.")
        endif()
    endif()
endfunction()

function(pa_configure NAME)
    get_target_property(TARGET_TYPE ${NAME} PA_DETAIL_TYPE)

    if ("${TARGET_TYPE}" STREQUAL "type-NOTFOUND")
        message(SEND_ERROR "Target '${TARGET}' was not defined using pa_add_xxx")
        return()
    endif()

    pa_detail_configure_parse(
        "${TARGET_TYPE}"
        ""
        "PLATFORM"
        ""
        ${ARGN}
    )

    string(REPLACE "::" "_" TARGET ${NAME})
    string(REPLACE "+" "_" TARGET ${TARGET})
    string(REPLACE "-" "_" TARGET ${TARGET})
    string(REPLACE "." "_" TARGET ${TARGET})

    get_target_property(PUBLIC_TYPE ${TARGET} TYPE)
    if (${PUBLIC_TYPE} STREQUAL INTERFACE_LIBRARY)
        set(PUBLIC_TYPE INTERFACE)
    else()
        set(PUBLIC_TYPE PUBLIC)
    endif()

    set(DO_CONFIGURE ON)
    if(DEFINED PA_OPT_PLATFORM)
        if(NOT "x${CMAKE_SYSTEM_NAME}" STREQUAL "x${PA_OPT_PLATFORM}")
            set(DO_CONFIGURE OFF)
        endif()
    endif()

    pa_detail_configure(${NAME} ${TARGET} ${PUBLIC_TYPE})
endfunction()

function(pa_add_library NAME)
    pa_detail_configure_parse(
        LIB
        ""
        ""
        ""
        ${ARGN}
    )

    set(ADD_LIBRARY_ARGS "")
    if("${PA_OPT_STATIC}")
        set(ADD_LIBRARY_ARGS STATIC)
    elseif("${PA_OPT_SHARED}")
        set(ADD_LIBRARY_ARGS SHARED)
    elseif("${PA_OPT_MODULE}")
        set(ADD_LIBRARY_ARGS MODULE)
    else()
        if(BUILD_SHARED_LIBS)
            set(ADD_LIBRARY_ARGS SHARED)
        else()
            set(ADD_LIBRARY_ARGS STATIC)
        endif()
    endif()

    set(PUBLIC_TYPE INTERFACE)
    set(PRIVATE_TYPE INTERFACE)

    if(DEFINED PA_OPT_SOURCES)
        set(PUBLIC_TYPE PUBLIC)
        set(PRIVATE_TYPE PRIVATE)
    else()
        list(APPEND ADD_LIBRARY_ARGS "INTERFACE")
    endif()

    pa_detail_add_target(add_library ${NAME} ${ADD_LIBRARY_ARGS})

    set_target_properties(${TARGET} PROPERTIES PA_DETAIL_TYPE LIB)
    if(${PRIVATE_TYPE} STREQUAL PRIVATE)
        target_link_libraries(${TARGET} PRIVATE pa_detail_cxx_options)
    endif()

    pa_detail_configure(${NAME} ${TARGET} ${PUBLIC_TYPE})
endfunction()

function(pa_add_executable NAME)
    pa_detail_configure_parse(
        EXE
        ""
        ""
        ""
        ${ARGN}
    )
    pa_detail_add_target(add_executable ${NAME})

    set_target_properties(${TARGET} PROPERTIES PA_DETAIL_TYPE EXE)
    target_link_libraries(${TARGET} PRIVATE pa_detail_cxx_options)

    pa_detail_configure(${NAME} ${TARGET} PUBLIC)
endfunction()
