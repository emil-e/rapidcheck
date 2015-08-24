option(RC_ENABLE_CATCH "Build Catch.hpp support" OFF)
if (RC_ENABLE_CATCH OR RC_ENABLE_TESTS)
  add_subdirectory(catch)
endif()

option(RC_ENABLE_GMOCK "Build Google Mock integration" OFF)
if (RC_ENABLE_GMOCK)
  add_subdirectory(gmock)
endif()

option(RC_ENABLE_GTEST "Build Google Test integration" OFF)
if (RC_ENABLE_GTEST)
  add_subdirectory(gtest)
endif()

option(RC_ENABLE_BOOST "Build Boost support" OFF)
if (RC_ENABLE_BOOST)
  add_subdirectory(boost)
endif()

option(RC_ENABLE_BOOST_TEST "Build Boost Test support" OFF)
if (RC_ENABLE_BOOST_TEST)
  add_subdirectory(boost_test)
endif()
