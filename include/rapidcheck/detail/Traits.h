#pragma once

#include <type_traits>
#include <iostream>

namespace rc {
namespace detail {

//! Convenience wrapper over std::decay
template<typename T>
using DecayT = typename std::decay<T>::type;

namespace test {

template<typename T, typename = decltype(std::declval<T>() == std::declval<T>())>
std::true_type isEqualityComparable(const T &);
std::false_type isEqualityComparable(...);

template<typename T, typename = decltype(std::cout << std::declval<T>())>
std::true_type isStreamInsertible(const T &);
std::false_type isStreamInsertible(...);

} // namespace test

template<typename T>
using IsEqualityComparable = decltype(
    test::isEqualityComparable(std::declval<T>()));

template<typename T>
using IsStreamInsertible = decltype(
    test::isStreamInsertible(std::declval<T>()));

} // namespace detail
} // namespace rc
