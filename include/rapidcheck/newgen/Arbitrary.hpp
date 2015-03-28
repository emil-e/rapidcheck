#pragma once

namespace rc {
namespace newgen {
namespace detail {

template<typename T>
struct DefaultArbitrary;

} // namespace detail

template<typename T>
decltype(NewArbitrary<T>::arbitrary()) arbitrary()
{
    static const auto instance = rc::NewArbitrary<T>::arbitrary();
    return instance;
}

} // namespace newgen

template<typename T>
struct NewArbitrary
{
    static decltype(newgen::detail::DefaultArbitrary<T>::arbitrary()) arbitrary()
    { return newgen::detail::DefaultArbitrary<T>::arbitrary(); }
};

} // namespace rc

#include "Arbitrary.hpp"
