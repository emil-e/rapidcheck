#pragma once

namespace rc {
namespace detail {

template<typename Testable, typename ...Args>
TestResult newCheckTestable(Testable &&testable, const Args &...args)
{
    return checkProperty(
        toNewProperty(std::forward<Testable>(testable)),
        args...);
}

} // namespace detail
} // namespace rc
