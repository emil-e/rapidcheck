#pragma once

namespace rc {
namespace gen {

template <typename T>
Gen<T> nonEmpty(Gen<T> gen) {
  return gen::suchThat(std::move(gen), [](const T &x) { return !x.empty(); });
}

template <typename T>
Gen<T> nonEmpty() {
  return gen::suchThat(gen::arbitrary<T>(),
                       [](const T &x) { return !x.empty(); });
}

} // namespace gen
} // namespace rc
