#pragma once

#include <cstddef>

#include "rapidcheck/detail/RandomEngine.h"

namespace rc {

namespace gen {
class ValueDescription;
} // namespace gen

namespace detail {

//! Describes a particular test case.
struct TestCase
{
    //! The test case index.
    int index = 0;
    //! The used size.
    size_t size = 0;
    //! The used seed.
    RandomEngine::Atom seed = 0;
};

//! The result of a test
enum class Result { Success, Failure };

//! Describes the circumstances around the result of a test.
struct TestResults
{
    //! The result of the test.
    Result result = Result::Success;
    //! If the property failes, the failing test case.
    TestCase failingCase;
    //! The total number of shrinks.
    int numShrinks = 0;
    //! On failure, a vector of `ValueDescription`s describing the failing test
    //! case.
    std::vector<gen::ValueDescription> counterExample;
};

std::ostream &operator<<(std::ostream &os, Result result);

} // namespace detail
} // namespace rc
