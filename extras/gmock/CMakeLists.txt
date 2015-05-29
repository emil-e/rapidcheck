add_library(rapidcheck_gmock INTERFACE)
target_link_libraries(rapidcheck_gmock
  INTERFACE rapidcheck gmock catch)
target_include_directories(rapidcheck_gmock
  INTERFACE
    include
    ${CMAKE_SOURCE_DIR}/ext/gmock/include
    ${CMAKE_SOURCE_DIR}/ext/gmock/gtest/include)

if (RC_BUILD_TESTS)
  add_subdirectory(test)
endif()