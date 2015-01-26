#pragma once

#include "detail/Property.h"

namespace rc {
namespace detail {

template<typename Testable>
TestResult checkTestable(const Testable &testable,
                         const TestParams &params)
{ return checkProperty(toProperty(testable), params); }

} // namespace detail
} // namespace rc
