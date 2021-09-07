add_library(sqlite3 STATIC "src/sqlite3.c")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")  # clang-cl
        target_compile_options(sqlite3 PRIVATE -Wno-deprecated)
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")  # clang
        target_compile_options(sqlite3 PRIVATE -Wno-deprecated)
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif()
