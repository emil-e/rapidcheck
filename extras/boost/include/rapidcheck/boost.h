#pragma once

namespace rc {

template <typename T>
struct Arbitrary<boost::optional<T>> {
  static Gen<boost::optional<T>> arbitrary() {
    return gen::map<Maybe<T>>([](Maybe<T> &&m) -> boost::optional<T> {
      if (m) {
        return *m;
      } else {
        return boost::none;
      }
    });
  }
};

} // namespace rc
