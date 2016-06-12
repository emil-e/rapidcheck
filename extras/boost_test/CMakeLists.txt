add_library(rapidcheck_boost_test INTERFACE)
target_link_libraries(rapidcheck_boost_test INTERFACE rapidcheck)
target_include_directories(rapidcheck_boost_test INTERFACE include)

install(DIRECTORY "include/"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
  )

if(RC_ENABLE_INSTALL)

	install(DIRECTORY "include/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
		)

endif()
