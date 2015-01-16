#pragma once

#include "ValueDescription.h"

namespace rc {
namespace detail {

//! Variant class that can hold a value of any type.
class Any {
public:
    //! Constructs a new null `Any`.
    Any();

    //! Constructs a new `Any` with the given value.
    template<typename T>
    Any(T &&value);

    //! Assignment operator to copy or move a value of type `T` into this
    //! `Any`.
    template<typename T>
    Any &operator=(T &&rhs);

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

    //! Returns a string representation of this `Any`.
    ValueDescription describe() const;

    //! Returns `true` if this `Any` is non-null.
    operator bool() const;

    Any(Any &&other);
    Any &operator=(Any &&rhs);
    ~Any();

private:
    RC_DISABLE_COPY(Any)

    void *m_value;
    void (*m_delete)(void *);
    ValueDescription (*m_describe)(void *);
};

} // namespace detail
} // namespace rc

#include "Any.hpp"
