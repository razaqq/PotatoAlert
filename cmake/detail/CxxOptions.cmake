add_library(pa_detail_cxx_options INTERFACE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")  # clang-cl
        target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:DEBUG>:/W4 /Ob0 /Od>)
        target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
    elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")  # clang
        target_compile_options(pa_detail_cxx_options INTERFACE -fno-exceptions)
        target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:DEBUG>:-Wall -pedantic -Wno-unknown-pragmas -O0>)
        target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:RELEASE>:-O3>)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:DEBUG>:/W4 /Od /Ob0>)  # TODO: completely untested
    target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:RELEASE>:/O2 /Ob2>)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(pa_detail_cxx_options INTERFACE -fno-exceptions -fms-extensions -Wno-attributes)
    target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:DEBUG>:-Wall -pedantic -Wno-unknown-pragmas -O0>)
    target_compile_options(pa_detail_cxx_options INTERFACE $<$<CONFIG:RELEASE>:-O3>)
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()
