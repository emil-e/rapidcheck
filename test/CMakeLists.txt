add_executable(rapidcheck_tests
  AssertionsTests.cpp
  CheckTests.cpp
  SeqTests.cpp
  ShowTests.cpp
  ShrinkTests.cpp
  arbitrary/CollectionTests.cpp
  arbitrary/NumericTests.cpp
  detail/AnyTests.cpp
  detail/ConfigurationTests.cpp
  detail/ExpressionCaptorTests.cpp
  detail/ImplicitParamTests.cpp
  detail/MapParserTests.cpp
  detail/RandomEngineTests.cpp
  detail/ResultsTests.cpp
  detail/RoseTests.cpp
  detail/ShowTypeTests.cpp
  detail/VariantTests.cpp
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