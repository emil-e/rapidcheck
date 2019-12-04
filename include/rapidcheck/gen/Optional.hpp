#pragma once

namespace rc {
namespace gen {
namespace detail {

template <typename T>
struct DefaultArbitrary<std::optional<T>> {
  static Gen<std::optional<T>> arbitrary() {
    return gen::oneOf(
      gen::cast<std::optional<T>>(gen::arbitrary<T>()),
      gen::cast<std::optional<T>>(gen::element(std::nullopt)));
  }
};

} // namespace detail
} // namespace gen
} // namespace rc
