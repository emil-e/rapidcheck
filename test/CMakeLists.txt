add_executable(rapidcheck_tests
  AnyTests.cpp
  ArbitraryTests.cpp
  GeneratorTests.cpp
  RoseTests.cpp
  ShrinkTests.cpp
  VariantTests.cpp
  main.cpp
  )

target_link_libraries(rapidcheck_tests rapidcheck)
target_include_directories(rapidcheck_tests PRIVATE ../ext/catch/include)