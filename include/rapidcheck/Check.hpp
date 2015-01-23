#pragma once

#include "detail/Property.h"

namespace rc {
namespace detail {

template<typename Testable>
TestResult checkTestable(const Testable &testable)
{ return checkProperty(toProperty(testable)); }

} // namespace detail
} // namespace rc
