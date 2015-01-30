#pragma once

#include "detail/Property.h"

namespace rc {
namespace detail {

template<typename Testable, typename ...Args>
TestResult checkTestable(const Testable &testable, const Args &...args)
{ return checkProperty(toProperty(testable), args...); }

} // namespace detail
} // namespace rc
