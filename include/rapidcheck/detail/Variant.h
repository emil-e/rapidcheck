#pragma once

#include <cstdlib>
#include <iostream>

#include "rapidcheck/Traits.h"

namespace rc {
namespace detail {

//! `Variant` can contain any of the parameterized type but only one of them at
//! the same time. Allows functions to return different types.
//!
//! This class trades the simplicity of implementation for the requirement that
//! all types are no-throw move/copy constructible.
template<typename ...Types>
class Variant
{
public:
    //! Constructs a new `Variant` containing the specified value.
    template<typename T,
             typename = typename std::enable_if<
                 !std::is_same<Decay<T>, Variant>::value>::type>
    Variant(T &&value)
        noexcept(std::is_nothrow_constructible<Decay<T>, T &&>::value);

    //! Copy-assigns the given value to this `Variant`.
    template<typename T,
             typename = typename std::enable_if<
                 std::is_nothrow_move_constructible<T>::value>::type>
    Variant &operator=(const T &value) noexcept;

    //! Move-assigns the given value to this `Variant`.
    template<typename T,
             typename = typename std::enable_if<
                 !std::is_reference<T>::value &&
                 std::is_nothrow_constructible<Decay<T>, T &&>::value>::type>
    Variant &operator=(T &&value) noexcept;

    //! Returns `true` if this variant has type `T`.
    template<typename T>
    bool is() const;

    //! Returns a reference to the internal value as a reference to `T`.
    //! nchecked.
    template<typename T>
    T &get();

    //! Returns a reference to the internal value as a reference to `const T`.
    //! nchecked.
    template<typename T>
    const T &get() const;

    //! If this variant is of type `T`, assigns the value of the variant to
    //! `value` and return `true`.
    template<typename T>
    bool match(T &value) const;

    bool operator==(const Variant &rhs) const;
    void printTo(std::ostream &os) const;

    Variant(const Variant &other)
        noexcept(All<std::is_nothrow_copy_constructible, Types...>::value);
    Variant(Variant &&other)
        noexcept(All<std::is_nothrow_move_constructible, Types...>::value);

    Variant &operator=(const Variant &rhs)
        noexcept(All<std::is_nothrow_copy_constructible, Types...>::value &&
                 All<std::is_nothrow_copy_assignable, Types...>::value);

    Variant &operator=(Variant &&rhs)
        noexcept(All<std::is_nothrow_move_assignable, Types...>::value);

    ~Variant() noexcept;

private:
    static void copy(std::size_t index, void *to, const void *from);
    static void move(std::size_t index, void *to, void *from);
    static void copyAssign(std::size_t index, void *to, const void *from);
    static void moveAssign(std::size_t index, void *to, void *from);
    static void destroy(std::size_t index, void *storage) noexcept;

    template<typename T>
    static constexpr std::size_t indexOfType();

    template<typename T>
    static constexpr bool isValidType();

    std::size_t m_typeIndex;
    typedef typename std::aligned_union<0, Types...>::type Storage;
    Storage m_storage;
};

template<typename ...Types>
bool operator!=(const Variant<Types...> &lhs, const Variant<Types...> &rhs);

} // namespace detail
} // namespace rc

#include "Variant.hpp"
