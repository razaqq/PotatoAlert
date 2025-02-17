cmake_minimum_required(VERSION 3.17)

function(SetDllPath TARGET)
    # Right now cmake is not smart enough for this to work https://gitlab.kitware.com/cmake/cmake/-/issues/25686
    #set_tests_properties(
    #    ${TARGET}
    #    PROPERTIES
    #        ENVIRONMENT_MODIFICATION "PATH=path_list_append:$<TARGET_RUNTIME_DLL_DIRS:${TARGET}>")

    string(REPLACE "\\" "/" ESCAPED_PATH "$ENV{PATH}")
    set_tests_properties(${TARGET}
        PROPERTIES ENVIRONMENT "PATH=$<JOIN:$<TARGET_RUNTIME_DLL_DIRS:${TARGET}>;${ESCAPED_PATH},\\;>")
endfunction()

set(PA_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Data")

add_subdirectory(CoreTest)
add_subdirectory(GameFileUnpackTest)
add_subdirectory(GameTest)
add_subdirectory(ReplayTest)
