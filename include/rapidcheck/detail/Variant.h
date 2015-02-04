#pragma once

#include "Any.h"

namespace rc {
namespace detail {

template<typename ...Types>
class Variant;

template<typename ...Types>
bool operator==(const Variant<Types...> &v1, const Variant<Types...> &v2);

template<typename ...Types>
bool operator!=(const Variant<Types...> &v1, const Variant<Types...> &v2);

template<typename ...Types>
std::ostream &operator<<(std::ostream &os, const Variant<Types...> &variant);

//! `Variant` can contain any of the parameterized type but only one of them at
//! the same time. Allows functions to return different types.
template<typename ...Types>
class Variant
{
    friend bool operator==<Types...>(const Variant<Types...> &v1,
                                     const Variant<Types...> &v2);
    friend std::ostream &operator<<<Types...>(std::ostream &os,
                                              const Variant<Types...> &variant);

public:
    //! Constructs a new `Variant` containing the specified value.
    template<typename T>
    Variant(T &&value);

    //! Returns `true` if this variant has type `T`.
    template<typename T>
    bool is() const;

    //! If this variant is of type `T`, assigns the value of the variant to
    //! `value` and return `true`.
    template<typename T>
    bool match(T &value) const;

private:
    template<typename T>
    static constexpr std::ptrdiff_t indexOfType();

    template<typename T>
    static constexpr bool isValidType();

    std::ptrdiff_t m_typeIndex;
    Any m_value;
};

} // namespace detail
} // namespace rc

#include "Variant.hpp"
