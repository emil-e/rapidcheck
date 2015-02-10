add_executable(rapidcheck_tests
  AnyTests.cpp
  CheckTests.cpp
  ConfigurationTests.cpp
  ImplicitParamTests.cpp
  MapParserTests.cpp
  RandomEngineTests.cpp
  ResultsTests.cpp
  RoseTests.cpp
  ShowTests.cpp
  ShowTypeTests.cpp
  ShrinkTests.cpp
  VariantTests.cpp
  arbitrary/CollectionTests.cpp
  arbitrary/NumericTests.cpp
  gen/CollectionTests.cpp
  gen/CommonTests.cpp
  gen/DistinctTests.cpp
  gen/ElementTests.cpp
  gen/InvokeTests.cpp
  gen/NumericTests.cpp
  gen/OneOfTests.cpp
  gen/ParametersTests.cpp
  gen/TextTests.cpp
  gen/TupleTests.cpp
  main.cpp
  predicate/PredicatesTests.cpp
  state/StateTests.cpp
  )

target_link_libraries(rapidcheck_tests rapidcheck)
target_include_directories(rapidcheck_tests
  PRIVATE
  .
  ../ext/catch/include
  ../src)

add_test(
  NAME rapidcheck_tests
  COMMAND rapidcheck_tests)