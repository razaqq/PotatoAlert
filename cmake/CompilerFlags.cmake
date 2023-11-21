function(SetCompilerFlags target)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")  # clang-cl
            target_compile_options(${target} PRIVATE $<$<CONFIG:DEBUG>:/Wall -Wno-unknown-pragmas -Wno-error=unused-variable -Wno-c++98-compat -Wno-c++98-compat-pedantic /Ob0 /Od>)
            target_compile_options(${target} PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
        elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")  # clang
            target_compile_options(${target} PRIVATE -fno-exceptions)
            target_compile_options(${target} PRIVATE $<$<CONFIG:DEBUG>:-Wall -pedantic -Wno-unknown-pragmas -O0>)
            target_compile_options(${target} PRIVATE $<$<CONFIG:RELEASE>:-O3>)
        endif()
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE $<$<CONFIG:DEBUG>:/Wall /Od /Ob0>)  # TODO: completely untested
        target_compile_options(${target} PRIVATE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    else()
        message(FATAL_ERROR "Unsupported compiler!")
    endif()
endfunction()