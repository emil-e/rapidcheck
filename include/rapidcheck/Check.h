#pragma once

#include "detail/Results.h"

namespace rc {
namespace gen {

template<typename T> class Generator;

} // namespace gen

namespace detail {

//! Describes the parameters for a test.
struct TestParams
{
    //! The maximum number of successes before deciding a property passes.
    int maxSuccess = 100; // TODO dunno if I like the name, to be honest
    //! The maximum size to generate.
    int maxSize = 100;
    //! The maximum allowed number of discarded tests per successful test.
    int maxDiscardRatio = 10;
};

std::ostream &operator<<(std::ostream &os, const TestParams &params);

//! Checks the given property and returns the results.
TestResult checkProperty(const gen::Generator<CaseResult> &property,
                         const TestParams &params = TestParams());

//! Overload which first converts the testable to a property.
template<typename Testable>
TestResult checkTestable(const Testable &testable,
                         const TestParams &params = TestParams());

} // namespace detail
} // namespace rc

#include "Check.hpp"
