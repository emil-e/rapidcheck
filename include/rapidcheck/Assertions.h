#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Capture.h"

#define RC_INTERNAL_CONDITIONAL_RESULT(                                        \
    ResultType, expression, invert, name, ...)                                 \
  doAssert(RC_INTERNAL_CAPTURE(expression),                                    \
           (invert),                                                           \
           ::rc::detail::CaseResult::Type::ResultType,                         \
           __FILE__,                                                           \
           __LINE__,                                                           \
           name "(" #expression ")")

#define RC__UNCONDITIONAL_RESULT(ResultType, description)                      \
  do {                                                                         \
    throw ::rc::detail::CaseResult(::rc::detail::CaseResult::Type::ResultType, \
                                   ::rc::detail::makeDescriptionMessage(       \
                                       __FILE__, __LINE__, (description)));    \
  } while (false)

/// Fails the current test case unless the given condition is `true`.
#define RC_ASSERT(expression)                                                  \
  RC_INTERNAL_CONDITIONAL_RESULT(Failure, expression, true, "RC_ASSERT", )

/// Fails the current test case unless the given condition is `false`.
#define RC_ASSERT_FALSE(expression)                                            \
  RC_INTERNAL_CONDITIONAL_RESULT(Failure,                                      \
                                 expression,                                   \
                                 false,                                        \
                                 "RC_ASSERT_"                                  \
                                 "FALSE")

/// Unconditionally fails the current test case with the given message.
#define RC_FAIL(msg) RC__UNCONDITIONAL_RESULT(Failure, (msg))

/// Succeed if the given condition is true.
#define RC_SUCCEED_IF(expression)                                              \
  RC_INTERNAL_CONDITIONAL_RESULT(Success, expression, false, "RC_SUCCEED_IF")

/// Unconditionally succeed with the given message.
#define RC_SUCCEED(msg) RC__UNCONDITIONAL_RESULT(Success, (msg))

/// Discards the current test case if the given condition is false.
#define RC_PRE(expression)                                                     \
  RC_INTERNAL_CONDITIONAL_RESULT(Discard, expression, true, "RC_PRE", !)

/// Discards the current test case with the given description.
#define RC_DISCARD(msg) RC__UNCONDITIONAL_RESULT(Discard, (msg))

#include "Assertions.hpp"
