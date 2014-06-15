#pragma once

namespace rc {
namespace detail {

//! `Variant` can contain any of the parameterized type but only one of them at
//! the same time. Allows functions to return different types.
template<typename ...Types>
class Variant
{
public:
    //! Constructs a new `Variant` containing the specified value.
    template<typename T>
    Variant(T value);

    //! Copy constructor.
    Variant(const Variant &other);

    //! Move constructor.
    Variant(Variant &&other);

    //! Copy assignment.
    Variant &operator=(const Variant &rhs);

    //! Move assignment.
    Variant &operator=(Variant &&rhs);

    //! If this variant is of type `T`, assigns the value of the variant to
    //! `value` and return `true`.
    template<typename T>
    bool match(T &value) const;

    //! Returns `true` if this variant has type `T`.
    template<typename T>
    bool is() const;

    //! Destructor.
    ~Variant();

private:
    template<typename T>
    static constexpr int indexOfType();

    void *(*m_copy)(void *);
    void (*m_delete)(void *);
    int m_typeIndex;
    void *m_value;
};

} // namespace detail
} // namespace rc

#include "Variant.hpp"
