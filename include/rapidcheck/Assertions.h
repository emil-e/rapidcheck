#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/ExpressionCaptor.h"

#define RC__CAPTURE(expr) ((::rc::detail::ExpressionCaptor()->*expr).str())

#define RC_INTERNAL_CONDITIONAL_RESULT(                                        \
    ResultType, condition, name, expression)                                   \
  do {                                                                         \
    if (condition) {                                                           \
      throw ::rc::detail::CaseResult(                                          \
          ::rc::detail::CaseResult::Type::ResultType,                          \
          ::rc::detail::makeExpressionMessage(__FILE__,                        \
                                              __LINE__,                        \
                                              name "(" #expression ")",        \
                                              RC__CAPTURE(expression)));       \
    }                                                                          \
  } while (false)

#define RC__UNCONDITIONAL_RESULT(ResultType, description)                      \
  do {                                                                         \
    throw ::rc::detail::CaseResult(::rc::detail::CaseResult::Type::ResultType, \
                                   ::rc::detail::makeDescriptionMessage(       \
                                       __FILE__, __LINE__, (description)));    \
  } while (false)

/// Fails the current test case unless the given condition is `true`.
#define RC_ASSERT(expression)                                                  \
  RC_INTERNAL_CONDITIONAL_RESULT(                                              \
      Failure, !(expression), "RC_ASSERT", expression)

/// Fails the current test case unless the given condition is `false`.
#define RC_ASSERT_FALSE(expression)                                            \
  RC_INTERNAL_CONDITIONAL_RESULT(                                              \
      Failure, (expression), "RC_ASSERT_FALSE", expression)

/// Unconditionally fails the current test case with the given message.
#define RC_FAIL(msg) RC__UNCONDITIONAL_RESULT(Failure, (msg))

/// Succeed if the given condition is true.
#define RC_SUCCEED_IF(expression)                                              \
  RC_INTERNAL_CONDITIONAL_RESULT(                                              \
      Success, (expression), "RC_SUCCEED_IF", expression)

/// Unconditionally succeed with the given message.
#define RC_SUCCEED(msg) RC__UNCONDITIONAL_RESULT(Success, (msg))

/// Discards the current test case if the given condition is false.
#define RC_PRE(expression)                                                     \
  RC_INTERNAL_CONDITIONAL_RESULT(Discard, !(expression), "RC_PRE", expression)

/// Discards the current test case with the given description.
#define RC_DISCARD(msg) RC__UNCONDITIONAL_RESULT(Discard, (msg))

namespace rc {
namespace detail {

/// Creates a message for an assection macro with a description.
std::string makeDescriptionMessage(const std::string file,
                                   int line,
                                   const std::string &description);

/// Creates a message for an assertion macro with an expression.
std::string makeExpressionMessage(const std::string file,
                                  int line,
                                  const std::string &assertion,
                                  const std::string &expansion);

} // namespace detail
} // namespace rc
