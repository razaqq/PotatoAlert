if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(RUST_LIB_DIR "debug")
    set(CARGO_BUILD_TYPE "dev")
    if(WIN32)
        set(RUST_CC_ENV CFLAGS=-MDd CXXFLAGS=-MDd)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(RUST_LIB_DIR "release")
    set(CARGO_BUILD_TYPE "release")
    if(WIN32)
        set(RUST_CC_ENV CFLAGS=-MD CXXFLAGS=-MD)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(RUST_LIB_DIR "release-with-debug")
    set(CARGO_BUILD_TYPE "release-with-debug")
    if(WIN32)
        set(RUST_CC_ENV CFLAGS=-MD CXXFLAGS=-MD)
    endif()
endif()

if(LTO_ENABLED)
    message(STATUS "IPO / LTO enabled for rust")
    set(RUST_FLAGS "-Clinker-plugin-lto" "-Clinker=clang" "-Clink-arg=-fuse-ld=lld")
endif()

message(STATUS "ReplayAnalyzer CARGO_BUILD_TYPE set to ${CARGO_BUILD_TYPE}")
if(WIN32 AND LTO_ENABLED AND "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
    # have to set explicit target on win32 with clang-cl and proc macros when using LTO
    set(RUST_LIB_DIR "x86_64-pc-windows-msvc/${RUST_LIB_DIR}")
    set(CARGO_CMD cargo build --profile=${CARGO_BUILD_TYPE} --target x86_64-pc-windows-msvc)
else()
    set(CARGO_CMD cargo build --profile=${CARGO_BUILD_TYPE})
endif()

set(RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/ReplayAnalyzerRust.cpp")

add_library(ReplayAnalyzer STATIC ${RUST_PART_CXX})

if(WIN32)
    set(RUST_LIB "${CMAKE_CURRENT_BINARY_DIR}/${RUST_LIB_DIR}/ReplayAnalyzer.lib")
    target_link_libraries(ReplayAnalyzer PRIVATE ${RUST_LIB} Userenv Ntdll)
elseif(UNIX)
    set(RUST_LIB "${CMAKE_CURRENT_BINARY_DIR}/${RUST_LIB_DIR}/libreplayanalyzer.a")
    target_link_libraries(ReplayAnalyzer PRIVATE ${RUST_LIB})
endif()

add_custom_command(
    OUTPUT ${RUST_PART_CXX} ${RUST_LIB}
    COMMAND ${CMAKE_COMMAND} -E env ${RUST_CC_ENV} CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} RUSTFLAGS="${RUST_FLAGS}" ${CARGO_CMD}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/Analyzer/src/lib.rs.cc ${RUST_PART_CXX}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/Analyzer/src/lib.rs.h ${CMAKE_CURRENT_BINARY_DIR}/ReplayAnalyzerRust.hpp
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/lib.rs"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(ReplayAnalyzer INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
