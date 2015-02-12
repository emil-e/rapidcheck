#pragma once

#include <type_traits>

#include "rapidcheck/Traits.h"

namespace rc {

//! This class implements lazy sequences using iterator semantics. This is
//! mainly used by RapidCheck to implement shrinking where it is not feasible to
//! materialize all of the possible shrinks at once. In particular, a Seq may be
//! infinite although that's not appropriate for shrinking, of course!
//!
//! A `Seq` object is constructed either as an empty sequence using the default
//! constructor or with an implementation object that implements the actual
//! sequence.
//!
//! The implementation class must meet the following requirements:
//!   - It must provide a method `T next()` which returns the next value.
//!   - It must provide a method `bool hasNext() const` which returns `true` if
//!     there is a next value or `false` if there is not. Since this method must
//!     be const, the implementation object needs to always be in a state where
//!     it is possible to tell if there is a next value or not.
//!   - It must have a copy constructor that produces a semantically identical
//!     copy. This means that it should provide equal values to the original.
//!
//! However, unless you have a reason to create your own implementation class,
//! you should just use the provided combinators in the `rc::seq` namespace to
//! construct your `Seq`s.
template<typename T>
class Seq
{
public:
    //! The type of the values of this `Seq`.
    typedef T ValueType;

    //! Constructs an empty `Seq` that has no values.
    Seq() = default;

    //! Constructs a `Seq` from the given implementation object.
    template<typename Impl,
             typename = typename std::enable_if<
                 !std::is_same<Decay<Impl>, Seq>::value>::type>
    Seq(Impl &&impl);

    //! Returns `true` if there are more values, `false` if there are no more
    //! values.
    operator bool() const;

    //! Returns the next value.
    T next();

    Seq(const Seq &other);
    Seq &operator=(const Seq &rhs);
    Seq(Seq &&other);
    Seq &operator=(Seq &&rhs);

private:
    class ISeqImpl;

    template<typename Impl>
    class SeqImpl;

    std::unique_ptr<ISeqImpl> m_impl;
};

} // namespace rc

#include "Seq.hpp"
