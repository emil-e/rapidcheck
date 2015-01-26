add_executable(rapidcheck_tests
  AnyTests.cpp
  ArbitraryTests.cpp
  CheckTests.cpp
  GeneratorTests.cpp
  RandomEngineTests.cpp
  RoseTests.cpp
  ShrinkTests.cpp
  VariantTests.cpp
  ShowTests.cpp
  main.cpp
  )

target_link_libraries(rapidcheck_tests rapidcheck)
target_include_directories(rapidcheck_tests PRIVATE ../ext/catch/include)

add_test(
  NAME rapidcheck_tests
  COMMAND rapidcheck_tests)