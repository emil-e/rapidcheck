add_executable(gtest_integration main.cpp)
target_link_libraries(gtest_integration rapidcheck_gtest gtest)
target_include_directories(gtest_integration PRIVATE
  ${CMAKE_SOURCE_DIR}/ext/gmock/gtest/include)
