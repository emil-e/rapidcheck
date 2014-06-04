add_executable(rapidcheck_tests
  main.cpp
  ShrinkTests.cpp)

target_link_libraries(rapidcheck_tests rapidcheck)