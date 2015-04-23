#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/TestParams.h"

#include "rapidcheck/detail/Property.h"

namespace rc {
namespace detail {

//! Checks the given property using the given parameters and returns the
//! results.
TestResult checkProperty(const Property &property,
                         const TestParams &params = defaultTestParams());

//! Overload which first converts the testable to a property.
template<typename Testable, typename ...Args>
TestResult checkTestable(Testable &&testable, const Args &...args);

} // namespace detail

//! Checks the given testable and returns `true` on success and `false` on
//! failure. This method will also print information about the testing to
//! stderr.
template<typename Testable>
bool check(Testable &&testable);

//! Same as `check(Testable &&)` but also takes a description of the property
//! that is being tested as the first parameter. This will be used in the
//! output.
template<typename Testable>
bool check(const std::string &description, Testable &&testable);

} // namespace rc

#include "Check.hpp"
