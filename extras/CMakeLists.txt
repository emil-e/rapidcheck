option(RC_ENABLE_CATCH "Build Catch.hpp support" OFF)
if (RC_ENABLE_CATCH OR RC_ENABLE_TESTS)
  add_subdirectory(catch)
endif()

option(RC_ENABLE_GMOCK "Build GMock integration" OFF)
if (RC_ENABLE_GMOCK)
  add_subdirectory(gmock)
endif()

option(RC_ENABLE_GTEST "Build GTest integration" OFF)
if (RC_ENABLE_GTEST)
  add_subdirectory(gtest)
endif()

option(RC_ENABLE_BOOST "Build boost support" OFF)
if (RC_ENABLE_BOOST)
  add_subdirectory(boost)
endif()
