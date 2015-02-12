#pragma once

#include <type_traits>
#include <iostream>

namespace rc {
namespace detail {
namespace sfinae {

template<typename T, typename = decltype(std::declval<T>() == std::declval<T>())>
std::true_type isEqualityComparable(const T &);
std::false_type isEqualityComparable(...);

template<typename T, typename = decltype(std::cout << std::declval<T>())>
std::true_type isStreamInsertible(const T &);
std::false_type isStreamInsertible(...);

} // namespace sfinae

template<typename T>
using IsEqualityComparable = decltype(
    sfinae::isEqualityComparable(std::declval<T>()));

template<typename T>
using IsStreamInsertible = decltype(
    sfinae::isStreamInsertible(std::declval<T>()));

} // namespace detail
} // namespace rc
