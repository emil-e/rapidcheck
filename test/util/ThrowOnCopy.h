#pragma once

#include <stdexcept>

namespace rc {
namespace test {

//! Test utility which throws on copy for testing exception safety.
struct ThrowOnCopy
{
    ThrowOnCopy(std::string s) : value(std::move(s)) {}

    ThrowOnCopy(const ThrowOnCopy &)
    {
        throw std::runtime_error("can't copy");
    }

    ThrowOnCopy &operator=(const ThrowOnCopy &)
    {
        throw std::runtime_error("can't copy");
    }

    ThrowOnCopy(ThrowOnCopy &&) = default;
    ThrowOnCopy &operator=(ThrowOnCopy &&) = default;

    std::string value;
};

inline bool operator==(const ThrowOnCopy &lhs, const ThrowOnCopy &rhs)
{
    return lhs.value == rhs.value;
}

} // namespace test
} // namespace rc
