#pragma once

namespace rc {
namespace detail {

template<typename Testable, typename ...Args>
TestResult checkTestable(Testable &&testable, const Args &...args)
{
    return checkProperty(
        toProperty(std::forward<Testable>(testable)),
        args...);
}

} // namespace detail
} // namespace rc
