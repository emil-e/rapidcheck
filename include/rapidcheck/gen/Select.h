#pragma once

namespace rc {
namespace gen {

/// Returns a generator which randomly selects an element from the given
/// container. The container must support `begin(Container)` and
/// `end(Container)` and the returned iterator must be a random access iterator.
template <typename Container>
Gen<typename Decay<Container>::value_type> elementOf(Container &&container);

/// Returns a generator which randomly selects one of the given elements.
template <typename T, typename... Ts>
Gen<Decay<T>> element(T &&element, Ts &&... elements);

/// Returns a generator which randomly generates using one of the specified
/// generators.
template <typename T, typename... Ts>
Gen<T> oneOf(Gen<T> gen, Gen<Ts>... gens);

} // namespace gen
} // namespace rc

#include "Select.hpp"
