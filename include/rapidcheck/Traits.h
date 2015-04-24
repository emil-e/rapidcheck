#pragma once

#include <type_traits>

namespace rc {

/// Convenience wrapper over std::decay, shorter to type.
template<typename T>
using Decay = typename std::decay<T>::type;

/// Checks that all the parameters are true.
template<bool ...Xs>
struct AllTrue;

template<>
struct AllTrue<> : public std::true_type {};

template<bool X, bool ...Xs>
struct AllTrue<X, Xs...>
    : public std::integral_constant<
        bool, X && AllTrue<Xs...>::value> {};

/// Checks that all the types in `Ts` conforms to type trait `F`.
template<template <typename> class F, typename ...Ts>
struct AllIs : public AllTrue<F<Ts>::value...> {};

} // namespace rc
