add_executable(rapidcheck_gmock_tests main.cpp)
target_link_libraries(rapidcheck_gmock_tests rapidcheck_gmock)

# Prevent collision between Gmock and Catch
target_compile_definitions(rapidcheck_gmock_tests
  PRIVATE
    CATCH_CONFIG_PREFIX_ALL)