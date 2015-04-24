#pragma once

#include "rapidcheck/detail/BitStream.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/gen/Transform.h"

namespace rc {
namespace gen {
namespace detail {

template <typename T>
Shrinkable<T> integral(const Random &random, int size) {
  return shrinkable::shrinkRecur(
      rc::detail::bitStreamOf(random).nextWithSize<T>(size),
      &shrink::integral<T>);
}

extern template Shrinkable<char> integral<char>(const Random &random, int size);
extern template Shrinkable<unsigned char> integral<unsigned char>(
    const Random &random, int size);
extern template Shrinkable<short> integral<short>(
    const Random &random, int size);
extern template Shrinkable<unsigned short> integral<unsigned short>(
    const Random &random, int size);
extern template Shrinkable<int> integral<int>(const Random &random, int size);
extern template Shrinkable<unsigned int> integral<unsigned int>(
    const Random &random, int size);
extern template Shrinkable<long> integral<long>(const Random &random, int size);
extern template Shrinkable<unsigned long> integral<unsigned long>(
    const Random &random, int size);
extern template Shrinkable<long long> integral<long long>(
    const Random &random, int size);
extern template Shrinkable<unsigned long long> integral<unsigned long long>(
    const Random &random, int size);

template <typename T>
Shrinkable<T> real(const Random &random, int size) {
  // TODO this implementation sucks
  auto stream = rc::detail::bitStreamOf(random);
  double scale =
      std::min(size, kNominalSize) / static_cast<double>(kNominalSize);
  double a = stream.nextWithSize<int64_t>(size);
  double b =
      (stream.next<uint64_t>() * scale) / std::numeric_limits<uint64_t>::max();
  T value = a + b;
  return shrinkable::shrinkRecur(value, &shrink::real<T>);
}

extern template Shrinkable<float> real<float>(const Random &random, int size);
extern template Shrinkable<double> real<double>(const Random &random, int size);

Shrinkable<bool> boolean(const Random &random, int size);

template <typename T>
struct DefaultArbitrary {
  // If you ended up here, it means that RapidCheck wanted to generate an
  // arbitrary value of some type but you haven't declared a specialization of
  // Arbitrary for your type. Check the template stack trace to see which type
  // it is.
  static_assert(
      std::is_integral<T>::value, "No Arbitrary specialization for type T");

  static Gen<T> arbitrary() { return integral<T>; }
};

template <>
struct DefaultArbitrary<float> {
  static Gen<float> arbitrary() { return real<float>; }
};

template <>
struct DefaultArbitrary<double> {
  static Gen<double> arbitrary() { return real<double>; }
};

template <>
struct DefaultArbitrary<bool> {
  static Gen<bool> arbitrary() { return boolean; }
};

} // namespace detail

template <typename T>
Gen<T> inRange(T min, T max) {
  return [=](const Random &random, int size) {
    if (max <= min) {
      std::string msg;
      msg += "Invalid range [" + std::to_string(min);
      msg += ", " + std::to_string(max) + ")";
      throw GenerationFailure(msg);
    }

    const auto rangeSize =
        static_cast<Random::Number>(max) - static_cast<Random::Number>(min);
    const auto x = Random(random).next() % rangeSize;
    return shrinkable::just(static_cast<T>(x + min));
  };
}

template <typename T>
Gen<T> nonZero() {
  return gen::suchThat(gen::arbitrary<T>(), [](T x) { return x != 0; });
}

template <typename T>
Gen<T> positive() {
  return gen::suchThat(gen::arbitrary<T>(), [](T x) { return x > 0; });
}

template <typename T>
Gen<T> negative() {
  return gen::suchThat(gen::arbitrary<T>(), [](T x) { return x < 0; });
}

template <typename T>
Gen<T> nonNegative() {
  return gen::suchThat(gen::arbitrary<T>(), [](T x) { return x >= 0; });
}

} // namespace gen
} // namespace rc
