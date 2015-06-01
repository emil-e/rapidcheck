if (RC_BUILD_TESTS)
  add_library(catch INTERFACE)
  target_include_directories(catch INTERFACE catch/include)
endif()

if (RC_BUILD_GMOCK AND RC_BUILD_TESTS)
  if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/gmock/CMakeLists.txt")
    execute_process(
      COMMAND "sh" "get_gmock.sh"
      WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
  endif()
  add_subdirectory(gmock)
endif()