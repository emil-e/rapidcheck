#pragma once

namespace rc {

//! A `Shrinkable` describes a value in addition to all the possible ways of
//! shrinking that value.
//!
//! `Shrinkable` is implemented as a type erased implementation object which
//! must have the following:
//!   - A method `T value() const` which returns the value.
//!   - A method `Seq<Shrinkable<T>> shrinks() const` which returns a `Seq` of
//!     the possible shrinks.
//!   - A copy constructor which produces a functionally identical object.
template<typename T>
class Shrinkable
{
    template<typename Impl, typename ...Args>
    friend Shrinkable<Decay<decltype(std::declval<Impl>().value())>>
    makeShrinkable(Args &&...args);

public:
    //! The type of the value in this `Shrinkable`.
    typedef T ValueType;

    template<
        typename Impl,
        typename = typename std::enable_if<
            !std::is_same<Decay<Impl>, Shrinkable>::value>::type>
    Shrinkable(Impl &&impl);

    //! Returns the value.
    T value() const;

    //! Returns a `Seq` of all the possible shrinks of this `Shrinkable`.
    Seq<Shrinkable<T>> shrinks() const;

    Shrinkable(const Shrinkable &other);
    Shrinkable &operator=(const Shrinkable &rhs);
    Shrinkable(Shrinkable &&other) = default;
    Shrinkable &operator=(Shrinkable &&rhs) = default;

private:
    class IShrinkableImpl;

    explicit Shrinkable(std::unique_ptr<IShrinkableImpl> impl);

    template<typename Impl>
    class ShrinkableImpl;

    std::unique_ptr<IShrinkableImpl> m_impl;
};

//! Two `Shrinkable`s are equal if the have the same value and the same shrinks.
template<typename T>
bool operator==(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs);

template<typename T>
bool operator!=(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs);

} // namespace rc

#include "Shrinkable.hpp"
