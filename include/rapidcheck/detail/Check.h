#pragma once

#define RC_CONDITIONAL_RESULT(ResultType, condition, msg)              \
    ::rc::detail::throwResultIf(                                       \
        ::rc::detail::CaseResult::Type::ResultType,                    \
        (condition),                                                   \
        msg,                                                           \
        __FILE__,                                                      \
        __LINE__)

namespace rc {
namespace detail {

//! Checks the given property and returns the results.
TestResult checkProperty(const gen::Generator<CaseResult> &property);

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

} // namespace detail
} // namespace rc
