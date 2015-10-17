add_library(rapidcheck_test_utils
  util/ArbitraryRandom.cpp
  util/GenUtils.cpp
  util/ShrinkableUtils.cpp)

target_link_libraries(rapidcheck_test_utils PUBLIC
  rapidcheck)
target_include_directories(rapidcheck_test_utils PUBLIC
  .)

add_executable(rapidcheck_tests
  AssertionsTests.cpp
  CheckTests.cpp
  ClassifyTests.cpp
  GenTests.cpp
  LogTests.cpp
  MaybeTests.cpp
  RandomTests.cpp
  SeqTests.cpp
  ShowTests.cpp
  ShrinkableTests.cpp
  detail/AnyTests.cpp
  detail/ApplyTupleTests.cpp
  detail/Base64Tests.cpp
  detail/BitStreamTests.cpp
  detail/CaptureTests.cpp
  detail/ConfigurationTests.cpp
  detail/FrequencyMapTests.cpp
  detail/ImplicitParamTests.cpp
  detail/LogTestListenerTests.cpp
  detail/MapParserTests.cpp
  detail/MulticastTestListenerTests.cpp
  detail/PropertyTests.cpp
  detail/ResultsTests.cpp
  detail/SerializationTests.cpp
  detail/ShowTypeTests.cpp
  detail/TestMetadataTests.cpp
  detail/TestParamsTests.cpp
  detail/TestingTests.cpp
  detail/VariantTests.cpp
  fn/CommonTests.cpp
  gen/BuildTests.cpp
  gen/ChronoTests.cpp
  gen/ContainerTests/Fixed.cpp
  gen/ContainerTests/NonFixed.cpp
  gen/ContainerTests/Unique.cpp
  gen/CreateTests.cpp
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
  state/CommandTests.cpp
  state/CommandsTests.cpp
  state/IntegrationTests.cpp
  state/StateTests.cpp
  state/gen/CommandsTests.cpp
  )

target_link_libraries(rapidcheck_tests
  rapidcheck
  catch
  rapidcheck_catch
  rapidcheck_test_utils)
target_include_directories(rapidcheck_tests PRIVATE
  .
  ../src)

add_test(
  NAME rapidcheck_tests
  COMMAND rapidcheck_tests)
