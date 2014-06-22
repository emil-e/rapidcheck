add_executable(rapidcheck_tests
  main.cpp
  ShrinkTests.cpp
  GeneratorTests.cpp
  ArbitraryTests.cpp)

target_link_libraries(rapidcheck_tests rapidcheck)
target_include_directories(rapidcheck_tests PRIVATE ext/catch/include)