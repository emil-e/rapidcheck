add_executable(rapidcheck_boost_tests
  main.cpp
  OptionalTests.cpp
  )

target_link_libraries(rapidcheck_boost_tests
  boost
  rapidcheck_boost
  rapidcheck_test_utils
  catch)

add_test(
  NAME rapidcheck_boost_tests
  COMMAND rapidcheck_boost_tests)