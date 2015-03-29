#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/TestParams.h"

namespace rc {
namespace gen {

template<typename T> class Generator;

} // namespace gen

namespace detail {

//! Checks the given property using the given parameters and returns the
//! results.
TestResult checkProperty(const gen::Generator<CaseResult> &property,
                         const TestParams &params = defaultTestParams());

//! Overload which first converts the testable to a property.
template<typename Testable, typename ...Args>
TestResult checkTestable(const Testable &testable, const Args &...args);

} // namespace detail
} // namespace rc

#include "Check.hpp"
