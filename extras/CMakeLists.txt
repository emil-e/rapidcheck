if (RC_ENABLE_CATCH OR RC_ENABLE_TESTS)
  add_subdirectory(catch)
endif()

if (RC_ENABLE_GMOCK)
  add_subdirectory(gmock)
endif()

if (RC_ENABLE_BOOST)
  add_subdirectory(boost)
endif()