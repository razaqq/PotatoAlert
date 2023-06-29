if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SSL_DLLS
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/libssl-1_1-x64.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto-1_1-x64.dll"
            CACHE INTERNAL ""
        )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(SSL_DLLS
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/libssl-1_1.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto-1_1.dll"
            CACHE INTERNAL ""
        )
    endif()
endif()
