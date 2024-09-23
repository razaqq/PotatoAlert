add_library(OpenSSL INTERFACE)

if (WIN32)
	target_link_libraries(OpenSSL
		INTERFACE
			debug ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/libcrypto-d.lib
			optimized ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/libcrypto.lib
	)
	target_include_directories(OpenSSL INTERFACE include)
elseif (UNIX)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(OpenSSL INTERFACE ${OPENSSL_CRYPTO_LIBRARY})
	target_include_directories(OpenSSL INTERFACE ${OPENSSL_INCLUDE_DIR})
endif ()
