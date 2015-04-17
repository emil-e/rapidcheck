#pragma once

#include <type_traits>

namespace rc {

//! Convenience wrapper over std::decay, shorter to type.
template<typename T>
using Decay = typename std::decay<T>::type;

//! Checks that all the types in `Ts` conforms to type trait `F`.
template<template <typename> class F, typename ...Ts>
struct All;

template<template <class> class F>
struct All<F> : public std::true_type {};

template<template <class> class F, typename T, typename ...Ts>
struct All<F, T, Ts...>
    : public std::integral_constant<
        bool, F<T>::value && All<F, Ts...>::value> {};

} // namespace rc
