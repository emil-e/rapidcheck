#pragma once

namespace rc {
namespace gen {

/// Generates a value `x` using the given generator for `x.empty()` is false.
/// Useful with strings, STL containers and other types.
template <typename T>
Gen<T> nonEmpty(Gen<T> gen);

/// Same as `Gen<T> nonEmpty(Gen<T>)` but uses `gen::arbitrary` for the
/// underlying generator.
template <typename T>
Gen<T> nonEmpty();

} // namespace gen
} // namespace rc

#include "Predicate.hpp"
