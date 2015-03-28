#pragma once

#include "rapidcheck/Seq.h"

namespace rc {

//! A `Shrinkable` describes a value in addition to all the possible ways of
//! shrinking that value.
//!
//! `Shrinkable` is backed by a type erased implementation object which must
//! have the following:
//!   - A method `T value() const` which returns the value.
//!   - A method `Seq<Shrinkable<T>> shrinks() const` which returns a `Seq` of
//!     the possible shrinks.
//!
//! A Shrinkable is immutable and the implementation object is shared when the
//! shrinkable is copied which is why the implementation object needs no copy
//! constructor.
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

private:
    class IShrinkableImpl;

    explicit Shrinkable(std::shared_ptr
                        <IShrinkableImpl> impl);

    template<typename Impl>
    class ShrinkableImpl;

    std::shared_ptr<const IShrinkableImpl> m_impl;
};

//! Two `Shrinkable`s are equal if the have the same value and the same shrinks.
template<typename T>
bool operator==(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs);

template<typename T>
bool operator!=(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs);

} // namespace rc

#include "Shrinkable.hpp"
