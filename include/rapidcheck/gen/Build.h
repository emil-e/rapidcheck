#pragma once

namespace rc {
namespace gen {

/// Returns a generator which generates objects of type `T` by constructing them
/// with values from the given generators.
template <typename T, typename... Args>
Gen<T> construct(Gen<Args>... gens);

/// Same as `gen::construct(Gen<Args>...)` but uses `gen::arbitrary` for all
/// types.
template <typename T, typename Arg, typename... Args>
Gen<T> construct();

} // namespace gen
} // namespace rc

#include "Build.hpp"
