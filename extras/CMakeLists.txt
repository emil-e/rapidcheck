# Since 
option(RC_INSTALL_ALL_EXTRAS "Add all possible integrations without
  requiring the initialization of all the submodules in ext." OFF)

option(RC_ENABLE_CATCH "Build Catch.hpp support" OFF)
if (RC_ENABLE_CATCH OR RC_ENABLE_TESTS OR RC_INSTALL_ALL_EXTRAS)
  add_subdirectory(catch)
endif()

option(RC_ENABLE_GMOCK "Build Google Mock integration" OFF)
if (RC_ENABLE_GMOCK OR RC_INSTALL_ALL_EXTRAS)
  add_subdirectory(gmock)
endif()

option(RC_ENABLE_GTEST "Build Google Test integration" OFF)
if (RC_ENABLE_GTEST OR RC_INSTALL_ALL_EXTRAS)
  add_subdirectory(gtest)
endif()

option(RC_ENABLE_BOOST "Build Boost support" OFF)
if (RC_ENABLE_BOOST OR RC_INSTALL_ALL_EXTRAS)
  add_subdirectory(boost)
endif()

option(RC_ENABLE_BOOST_TEST "Build Boost Test support" OFF)
if (RC_ENABLE_BOOST_TEST OR RC_INSTALL_ALL_EXTRAS)
  add_subdirectory(boost_test)
endif()
