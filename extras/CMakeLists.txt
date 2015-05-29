option(RC_BUILD_GMOCK "Build GMock integration" OFF)

if (RC_BUILD_GMOCK)
  add_subdirectory(gmock)
endif()