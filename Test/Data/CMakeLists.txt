set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

function(CopyTestDir target dir)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${TEST_DATA_DIR}/${dir}
        $<TARGET_FILE_DIR:${target}>/${dir}
    )
endfunction()
