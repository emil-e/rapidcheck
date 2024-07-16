#pragma once

namespace rc {
namespace detail {

template <std::size_t... Vs>
struct MaxOf;

template <std::size_t V>
struct MaxOf<V> : public std::integral_constant<std::size_t, V> {};

template <std::size_t V1, std::size_t V2, std::size_t... Vs>
struct MaxOf<V1, V2, Vs...>
    : public std::integral_constant<
          std::size_t,
          (V1 > MaxOf<V2, Vs...>::value ? V1 : MaxOf<V2, Vs...>::value)> {};

/// Replacement for std::aligned_union for compiles that do not have it.
///
/// See https://stackoverflow.com/questions/71828288/why-is-stdaligned-storage-to-be-deprecated-in-c23-and-what-to-use-instead

template <typename... Ts>
struct alignas(MaxOf<alignof(Ts)...>::value) AlignedUnion
{
  std::uint8_t bytes [MaxOf<sizeof(Ts)...>::value];
};

} // namespace detail
} // namespace rc
