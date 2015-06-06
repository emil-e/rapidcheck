add_executable(rapidcheck_tests
  AssertionsTests.cpp
  CheckTests.cpp
  ClassifyTests.cpp
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
  detail/FrequencyMapTests.cpp
  detail/ImplicitParamTests.cpp
  detail/LogTestListenerTests.cpp
  detail/MapParserTests.cpp
  detail/PropertyTests.cpp
  detail/ResultsTests.cpp
  detail/ShowTypeTests.cpp
  detail/TestParamsTests.cpp
  detail/TestingTests.cpp
  detail/VariantTests.cpp
  fn/CommonTests.cpp
  gen/BuildTests.cpp
  gen/ContainerTests.cpp
  gen/CreateTests.cpp
  gen/DistinctTests.cpp
  gen/ExecTests.cpp
  gen/MaybeTests.cpp
  gen/NumericTests.cpp
  gen/PredicateTests.cpp
  gen/SelectTests.cpp
  gen/TextTests.cpp
  gen/TransformTests.cpp
  gen/TupleTests.cpp
  gen/detail/ExecRawTests.cpp
  gen/detail/RecipeTests.cpp
  gen/detail/ScaleIntegerTests.cpp
  gen/detail/ShrinkValueIteratorTests.cpp
  main.cpp
  seq/CreateTests.cpp
  seq/OperationsTests.cpp
  seq/SeqIteratorTests.cpp
  seq/TransformTests.cpp
  shrink/ShrinkTests.cpp
  shrinkable/CreateTests.cpp
  shrinkable/OperationsTests.cpp
  shrinkable/TransformTests.cpp
  state/StateTests.cpp
  util/ArbitraryRandom.cpp
  util/GenUtils.cpp
  util/ShrinkableUtils.cpp
  )

target_link_libraries(rapidcheck_tests rapidcheck catch)
target_include_directories(rapidcheck_tests PRIVATE . ../src)

add_test(
  NAME rapidcheck_tests
  COMMAND rapidcheck_tests)