#pragma once

#include "rapidcheck/Seq.h"
#include "rapidcheck/detail/PolymorphicStorage.h"

namespace rc {

//! A `Shrinkable` is an abstraction of a value that may be shrunk. A
//! `Shrinkable` provides the current value and a `Seq` of the possible shrinks
//! that value.
//!
//! `Shrinkable` is backed by a type erased implementation object which must
//! have the following:
//!   - A method `T value() const` which returns the value.
//!   - A method `Seq<Shrinkable<T>> shrinks() const` which returns a `Seq` of
//!     the possible shrinks. If this method throws, it is treated as if it had
//!     returned an empty `Seq`.
//!   - A copy constructor which produces a functionally identical object. This
//!     constructor may not throw.
//!
//! Neither the copy constructor or a possible move constructor may not throw
//! since `Shrinkable` itself must be nothrow move- and copy-constructible.
//!
//! Unless you have a specific reason not to, you should just use the provided
//! combinators in `rc::shrinkable` to create instances.
template<typename T>
class Shrinkable
{
    template<typename Impl, typename ...Args>
    friend Shrinkable<Decay<decltype(std::declval<Impl>().value())>>
    makeShrinkable(Args &&...args);

public:
    //! The type of the value in this `Shrinkable`.
    typedef T ValueType;

    //! Returns the value.
    T value() const;

    //! Returns a `Seq` of all the possible shrinks of this `Shrinkable`.
    Seq<Shrinkable<T>> shrinks() const noexcept;

private:
    explicit Shrinkable() = default;

    class IShrinkableImpl;

    template<typename Impl>
    class ShrinkableImpl;

    template<typename Impl>
    class SharedShrinkableImpl;

    typedef detail::PolymorphicStorage<sizeof(void *) * 4> Storage;
    Storage m_storage;
};

//! Two `Shrinkable`s are equal if the have the same value and the same shrinks.
template<typename T>
bool operator==(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs);

template<typename T>
bool operator!=(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs);

} // namespace rc

#include "Shrinkable.hpp"
