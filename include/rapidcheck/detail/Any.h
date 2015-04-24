#pragma once

#include <memory>

namespace rc {
namespace detail {

class ValueDescription;

/// Variant class that can hold a value of any type.
class Any
{
public:
    /// Constructs a new null `Any`.
    Any() noexcept;

    /// Constructs a new `Any` with the given value.
    template<typename T>
    static Any of(T &&value);

    /// Resets this `Any` to null.
    void reset();

    /// Returns a const reference to the contained value. No type checking is
    /// currently done so be careful!
    template<typename T>
    const T &get() const;

    /// Returns a reference to the contained value. No type checking is currently
    /// done so be careful!
    template<typename T>
    T &get();

    /// Returns a pair of the type name and a string representation of the
    /// value.
    std::pair<std::string, std::string> describe() const;

    /// Returns `true` if this `Any` is non-null.
    explicit operator bool() const;

    Any(Any &&other) noexcept = default;
    Any &operator=(Any &&rhs) noexcept = default;

private:
    class IAnyImpl;

    template<typename T>
    class AnyImpl;

    std::unique_ptr<IAnyImpl> m_impl;
};

std::ostream &operator<<(std::ostream &os, const Any &value);

} // namespace detail
} // namespace rc

#include "Any.hpp"
