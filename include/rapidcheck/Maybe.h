#pragma once

namespace rc {

//! Tag struct that can be used to construct a `Maybe` to an initialized state.
struct NothingType {
    //! Implicit conversion to false.
    operator bool() const { return false; }
};

//! Singleton NothingType value.
constexpr NothingType Nothing = NothingType();

//! Represents the presence of a value or nothing at all. Like `Maybe` in
//! Haskell, `Option` in Scala or `optional` in Boost. But we're not Haskell nor
//! Scala and we don't depend on Boost so we have our own.
template<typename T>
class Maybe
{
public:
    //! The type of value contained in this `Maybe`.
    typedef T ValueType;

    //! Constructs a new empty `Maybe`.
    Maybe() noexcept;

    //! Equivalent to default construction.
    Maybe(NothingType) noexcept;

    //! Constructs a new `Maybe` and copy-constructs its value.
    Maybe(const T &value);

    //! Constructs a new `Maybe` and move-constructs its value.
    Maybe(T &&value);

    //! Constructs a new `Maybe` and copy-constructs or copy-assigns its value.
    Maybe &operator=(const T &value);

    //! Constructs a new `Maybe` and move-constructs or move-assigns its value.
    Maybe &operator=(T &&value);

    //! Equivalent to `reset()`.
    Maybe &operator=(NothingType);

    //! Initializes this `Maybe` with an in-place constructed value that gets
    //! forwarded the given arguments.
    template<typename ...Args>
    void init(Args &&...args);

    //! Resets this `Maybe`, destroying the contained object.
    void reset();

    //! Returns a reference to the contained value. No checking is performed.
    T &operator*();

    //! Returns a const reference to the contained value. No checking is
    //! performed.
    const T &operator*() const;

    //! Returns a pointer to the contained value. No checking is performed.
    T *operator->();

    //! Returns a const pointer to the contained value. No checking is
    //! performed.
    const T *operator->() const;

    //! Returns true if this `Maybe` is initialized.
    operator bool() const;

    Maybe(const Maybe &other) noexcept(
        std::is_nothrow_copy_constructible<T>::value);
    Maybe &operator=(const Maybe &rhs) noexcept(
        std::is_nothrow_copy_constructible<T>::value &&
        std::is_nothrow_copy_assignable<T>::value);

    Maybe(Maybe &&other) noexcept(
        std::is_nothrow_move_constructible<T>::value);
    Maybe &operator=(Maybe &&rhs) noexcept(
        std::is_nothrow_move_constructible<T>::value &&
        std::is_nothrow_move_assignable<T>::value);

    ~Maybe();

private:
    typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type Storage;
    Storage m_storage;
    bool m_initialized;
};

template<typename T, typename U>
bool operator==(const Maybe<T> &lhs, const Maybe<U> &rhs);

template<typename T, typename U>
bool operator!=(const Maybe<T> &lhs, const Maybe<U> &rhs);

template<typename T>
std::ostream &operator<<(std::ostream &os, const Maybe<T> &value);

} // namespace rc

#include "Maybe.hpp"
