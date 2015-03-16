#pragma once

namespace rc {
namespace newgen {
namespace detail {

template<typename T>
struct DefaultArbitrary;

} // namespace detail

template<typename T>
Gen<T> arbitrary()
{
    static const Gen<T> instance = rc::NewArbitrary<T>::arbitrary();
    return instance;
}

} // namespace newgen

template<typename T>
struct NewArbitrary
{
    static Gen<T> arbitrary()
    { return newgen::detail::DefaultArbitrary<T>::arbitrary(); }
};

} // namespace rc

#include "Arbitrary.hpp"
