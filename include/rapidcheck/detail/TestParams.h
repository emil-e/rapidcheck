#pragma once

#include <iosfwd>

namespace rc {
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

} // namespace detail
} // namespace rc
