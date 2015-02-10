#pragma once

#include "rapidcheck/detail/Results.h"


#define RC__CONDITIONAL_RESULT(ResultType, condition, msg)      \
    ::rc::detail::throwResultIf(                                \
        ::rc::detail::CaseResult::Type::ResultType,             \
        (condition),                                            \
        msg,                                                    \
        __FILE__,                                               \
        __LINE__)

#define RC__UNCONDITIONAL_RESULT(ResultType, msg)       \
    ::rc::detail::throwResult(                          \
        ::rc::detail::CaseResult::Type::ResultType,     \
        msg,                                            \
        __FILE__,                                       \
        __LINE__)

//! Fails the current test case unless the given condition is `true`.
#define RC_ASSERT(condition)                    \
    RC__CONDITIONAL_RESULT(Failure,             \
                           !(condition),        \
                           #condition)

//! Unconditionally fails the current test case with the given message.
#define RC_FAIL(msg)                            \
    RC__UNCONDITIONAL_RESULT(Failure, (msg))

//! Succeed if the given condition is true.
#define RC_SUCCEED_IF(condition)                \
    RC__CONDITIONAL_RESULT(Success,             \
                           (condition),         \
                           #condition)

//! Unconditionally succeed with the given message.
#define RC_SUCCEED(msg)                         \
    RC__UNCONDITIONAL_RESULT(Success, (msg))

//! Discards the current test case if the given condition is false.
#define RC_PRE(condition)                       \
    RC__CONDITIONAL_RESULT(Discard,             \
                           !(condition),        \
                           #condition)

//! Discards the current test case with the given description.
#define RC_DISCARD(msg)                         \
    RC__UNCONDITIONAL_RESULT(Discard, (msg))

namespace rc {
namespace detail {

//! Throws a result of the given type if `condition` is false.
//!
//! @param type         The result type.
//! @param condition    The condition to check.
//! @param description  A description of the potential failure.
//! @param file         The file in which the failure occurs.
//! @param line         The line at which the failure occurs.
void throwResultIf(CaseResult::Type type,
                   bool condition,
                   std::string description,
                   std::string file,
                   int line);

//! Throws a result of the given type.
//!
//! @param type         The result type.
//! @param description  A description of the potential failure.
//! @param file         The file in which the failure occurs.
//! @param line         The line at which the failure occurs.
[[noreturn]] void throwResult(CaseResult::Type type,
                              std::string description,
                              std::string file,
                              int line);

} // namespace detail
} // namespace rc
