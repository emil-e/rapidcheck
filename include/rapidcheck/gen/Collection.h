#pragma once

namespace rc {
namespace gen {

template<typename Container, typename Gen> class Vector;
template<typename Container, typename Gen> class Collection;

//! Generates a collection of the given size and of type `T` using the given
//! generator.
//!
//! @param size  The size to generate.
//! @param gen   The generator to use.
//!
//! @tparam Container  The collection type.
//! @tparam Gen        The generator type.
template<typename Container, typename Gen>
Vector<Container, Gen> vector(std::size_t size, Gen gen);

//! Generates a collection of the given type using the given generator.
//!
//! @param gen  The generator to use.
//!
//! @tparam Container  The collection type.
//! @tparam Gen        The generator type.
template<typename Container, typename Gen>
Collection<Container, Gen> collection(Gen gen);

} // namespace gen
} // namespace rc

#include "Collection.hpp"
