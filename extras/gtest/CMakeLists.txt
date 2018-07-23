add_library(rapidcheck_gtest INTERFACE)
target_link_libraries(rapidcheck_gtest INTERFACE rapidcheck)
target_include_directories(rapidcheck_gtest INTERFACE include)

install(DIRECTORY "include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
	)

if(RC_ENABLE_INSTALL)

	install(DIRECTORY "include/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
		)

endif()
