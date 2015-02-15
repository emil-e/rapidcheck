#pragma once

#include <memory>

namespace rc {
namespace detail {

class ValueDescription;
// TODO rename to IAnyImpl
class AbstractAnyImpl;

//! Variant class that can hold a value of any type.
class Any
{
public:
    //! Constructs a new null `Any`.
    Any() noexcept;

    //! Constructs a new `Any` with the given value.
    template<typename T>
    static Any of(T &&value);

    //! Resets this `Any` to null.
    void reset();

    //! Returns a const reference to the contained value. No type checking is
    //! currently done so be careful!
    template<typename T>
    const T &get() const;

    //! Returns a reference to the contained value. No type checking is currently
    //! done so be careful!
    template<typename T>
    T &get();

    //! Returns a pair of the type name and a string representation of the
    //! value.
    std::pair<std::string, std::string> describe() const;

    //! Returns `true` if this `Any` is non-null.
    operator bool() const;

    //! Returns `true` if this `Any` is copyable. Non-copyable `Any` will throw
    //! an exception if a copy is attempted.
    //!
    //! Maybe copy should be explicit?
    bool isCopyable() const;

    //! This constructor is effectively `noexcept` iff the contained value has a
    //! copy constructor and if that copy constructor does not throw. If the
    //! value does not have a copy constructor, this constructor throws.
    Any(const Any &other);

    //! This method is effectively `noexcept` iff the contained value has a
    //! copy constructor and if that copy constructor does not throw. If the
    //! value does not have a copy constructor, this method throws.
    Any &operator=(const Any &rhs);

    Any(Any &&other) noexcept;
    Any &operator=(Any &&rhs) noexcept;

private:
    std::unique_ptr<AbstractAnyImpl> m_impl;
};

} // namespace detail
} // namespace rc

#include "Any.hpp"
