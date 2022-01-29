add_library(OpenSSL INTERFACE)
target_include_directories(OpenSSL
	INTERFACE
		include
)
target_link_libraries(OpenSSL
	INTERFACE
		debug ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto-d.lib
		optimized ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto.lib
)
