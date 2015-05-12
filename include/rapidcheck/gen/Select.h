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

/// Takes a list of elements paired with respective weights and returns a
/// generator which generates one of the elements according to the weights.
template <typename T>
Gen<T>
weightedElement(std::initializer_list<std::pair<std::size_t, T>> pairs);

/// Returns a generator which randomly generates using one of the specified
/// generators.
template <typename T, typename... Ts>
Gen<T> oneOf(Gen<T> gen, Gen<Ts>... gens);

/// Takes a list of generators paired with respective weights and returns a
/// generator which randomly uses one of the generators according to the weights
/// to generate the final value.
template <typename T>
Gen<T>
weightedOneOf(std::initializer_list<std::pair<std::size_t, Gen<T>>> pairs);

} // namespace gen
} // namespace rc

#include "Select.hpp"
