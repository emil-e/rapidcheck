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
    //! The seed to use.
    int seed = 0;
    //! The maximum number of successes before deciding a property passes.
    int maxSuccess = 100;
    //! The maximum size to generate.
    int maxSize = 100;
    //! The maximum allowed number of discarded tests per successful test.
    int maxDiscardRatio = 10;
};

bool operator==(const TestParams &p1, const TestParams &p2);
bool operator!=(const TestParams &p1, const TestParams &p2);

std::ostream &operator<<(std::ostream &os, const TestParams &params);

//! Returns the default test parameters. Usually taken from the configuration.
TestParams defaultTestParams();

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
