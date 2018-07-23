add_library(rapidcheck_catch INTERFACE)
target_link_libraries(rapidcheck_catch INTERFACE rapidcheck)
target_include_directories(rapidcheck_catch INTERFACE include)

install(DIRECTORY "include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
	)

if(RC_ENABLE_INSTALL)

	install(DIRECTORY "include/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
		)

endif()
