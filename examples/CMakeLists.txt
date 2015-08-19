add_subdirectory(counter)
add_subdirectory(mapparser)
add_subdirectory(database)
add_subdirectory(classify)

if (RC_ENABLE_GTEST)
  add_subdirectory(gtest)
endif()
