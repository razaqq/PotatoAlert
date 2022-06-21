add_library(OpenSSL INTERFACE)
target_include_directories(OpenSSL
	INTERFACE
		include
)

if(WIN32)
	target_link_libraries(OpenSSL
		INTERFACE
			debug ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/libcrypto-d.lib
			optimized ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/libcrypto.lib
	)
endif()

if(UNIX)
	target_link_libraries(OpenSSL
		INTERFACE
			debug ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libcrypto-d.a
			optimized ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libcrypto.a
			)
endif()
