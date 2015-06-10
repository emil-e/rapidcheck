add_library(rapidcheck_boost INTERFACE)
target_link_libraries(rapidcheck_boost INTERFACE rapidcheck)
target_include_directories(rapidcheck_boost INTERFACE include)

if (RC_ENABLE_TESTS)
  add_subdirectory(test)
endif()