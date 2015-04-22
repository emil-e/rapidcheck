add_executable(rapidcheck_tests
  AssertionsTests.cpp
  CheckTests.cpp
  GenTests.cpp
  MaybeTests.cpp
  RandomTests.cpp
  SeqTests.cpp
  ShowTests.cpp
  ShrinkableTests.cpp
  detail/AnyTests.cpp
  detail/ApplyTupleTests.cpp
  detail/BitStreamTests.cpp
  detail/ConfigurationTests.cpp
  detail/ExpressionCaptorTests.cpp
  detail/ImplicitParamTests.cpp
  detail/MapParserTests.cpp
  detail/PropertyTests.cpp
  detail/ResultsTests.cpp
  detail/ShowTypeTests.cpp
  detail/TestParamsTests.cpp
  detail/VariantTests.cpp
  fn/CommonTests.cpp
  gen/ContainerTests.cpp
  gen/CreateTests.cpp
  gen/DistinctTests.cpp
  gen/ExecTests.cpp
  gen/NumericTests.cpp
  gen/SelectTests.cpp
  gen/TextTests.cpp
  gen/TransformTests.cpp
  gen/TupleTests.cpp
  gen/detail/ExecRawTests.cpp
  gen/detail/RecipeTests.cpp
  gen/detail/ShrinkValueIteratorTests.cpp
  main.cpp
  seq/CreateTests.cpp
  seq/OperationsTests.cpp
  seq/TransformTests.cpp
  shrink/ShrinkTests.cpp
  shrinkable/CreateTests.cpp
  shrinkable/OperationsTests.cpp
  shrinkable/TransformTests.cpp
  state/StateTests.cpp
  util/ArbitraryRandom.cpp
  util/GenUtils.cpp
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