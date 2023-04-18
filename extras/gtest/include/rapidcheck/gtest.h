#pragma once

#include <rapidcheck.h>

#include "rapidcheck/detail/ExecFixture.h"

namespace rc {
namespace detail {

template <typename Testable>
void checkGTest(Testable &&testable) {
  const auto testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
  TestMetadata metadata;
  metadata.id = std::string(testInfo->test_case_name()) + "/" +
      std::string(testInfo->name());
  metadata.description = std::string(testInfo->name());

  const auto result = checkTestable(std::forward<Testable>(testable), metadata);

  if (result.template is<SuccessResult>()) {
    const auto success = result.template get<SuccessResult>();
    if (!success.distribution.empty()) {
      printResultMessage(result, std::cout);
      std::cout << std::endl;
    }
  } else {
    std::ostringstream ss;
    printResultMessage(result, ss);
    FAIL() << ss.str() << std::endl;
  }
}

} // namespace detail
} // namespace rc

/// Defines a RapidCheck property as a Google Test.
#define RC_GTEST_PROP(TestCase, Name, ArgList)                                 \
  void rapidCheck_propImpl_##TestCase##_##Name ArgList;                        \
                                                                               \
  TEST(TestCase, Name) {                                                       \
    ::rc::detail::checkGTest(&rapidCheck_propImpl_##TestCase##_##Name);        \
  }                                                                            \
                                                                               \
  void rapidCheck_propImpl_##TestCase##_##Name ArgList

/// Defines a RapidCheck property as a Google Test fixture based test. The
/// fixture is reinstantiated for each test case of the property.
#define RC_GTEST_FIXTURE_PROP(Fixture, Name, ArgList)                          \
  class RapidCheckPropImpl_##Fixture##_##Name : public Fixture {               \
  public:                                                                      \
    void rapidCheck_fixtureSetUp() { SetUp(); }                                \
    void TestBody() override {}                                                \
    void operator() ArgList;                                                   \
    void rapidCheck_fixtureTearDown() { TearDown(); }                          \
  };                                                                           \
                                                                               \
  TEST(Fixture##_RapidCheck, Name) {                                           \
    ::rc::detail::checkGTest(&rc::detail::ExecFixture<                         \
                             RapidCheckPropImpl_##Fixture##_##Name>::exec);    \
  }                                                                            \
                                                                               \
  void RapidCheckPropImpl_##Fixture##_##Name::operator() ArgList

/// Helper macro for defining the RC_GTEST_* assertion macros.
#define RC_WRAP_GTEST_ASSERTION_IMPL(assertion, gtest_assertion, expression)
  do {
    try {
      assertion(expression);
    } catch(const ::rc::detail::CaseResult& e) {
      using rc::detail::param::CurrentPropertyContext;
      if(rc::detail::ImplicitParam<CurrentPropertyContext>::value() ==
         CurrentPropertyContext::defaultValue()) {
        gtest_assertion() << e.description;
      } else {
        throw;
      }
    }
  } while (false)

/// Wrapper for RC_ASSERT that also works in non-RC GTest tests.
#define RC_GTEST_ASSERT(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_ASSERT, FAIL, expression)

/// Wrapper for RC_ASSERT_FALSE that also works in non-RC GTest tests.
#define RC_GTEST_ASSERT_FALSE(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_ASSERT_FALSE, FAIL, expression)

/// Wrapper for RC_ASSERT_THROWS that also works in non-RC GTest tests.
#define RC_GTEST_ASSERT_THROWS(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_ASSERT_THROWS, FAIL, expression)

/// Wrapper for RC_ASSERT_THROWS_AS that also works in non-RC GTest tests.
#define RC_GTEST_ASSERT_THROWS_AS(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_ASSERT_THROWS_AS, FAIL, expression)

/// Wrapper for RC_ASSERT_FAIL that also works in non-RC GTest tests.
#define RC_GTEST_FAIL(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_FAIL, FAIL, expression)

/// Wrapper for RC_ASSERT_SUCCEED_IF that also works in non-RC GTest tests.
#define RC_GTEST_SUCCEED_IF(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_SUCCEED_IF, SUCCEED, expression)

/// Wrapper for RC_ASSERT_SUCCEED that also works in non-RC GTest tests.
#define RC_GTEST_SUCCEED(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_SUCCEED, SUCCEED, expression)

/// Wrapper for RC_PRE that also works in non-RC GTest tests.
#define RC_GTEST_PRE(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_PRE, FAIL, expression)

/// Wrapper for RC_DISCARD that also works in non-RC GTest tests.
#define RC_GTEST_DISCARD(expression)
  RC_WRAP_GTEST_ASSERTION_IMPL(RC_DISCARD, FAIL, expression)
