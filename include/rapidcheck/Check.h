#pragma once

#include "detail/Results.h"

namespace rc {
namespace gen {

template<typename T> class Generator;

} // namespace gen

namespace detail {

//! Checks the given property and returns the results.
TestResult checkProperty(const gen::Generator<CaseResult> &property);

//! Overload which first converts the testable to a property.
template<typename Testable>
TestResult checkTestable(const Testable &testable);

} // namespace detail
} // namespace rc

#include "Check.hpp"
