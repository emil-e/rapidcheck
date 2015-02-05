#pragma once

#include "rapidcheck/arbitrary/Arbitrary.h"

namespace rc {
namespace gen {

template<typename T> class Constant;
template<typename Gen, typename Predicate> class SuchThat;
template<typename Gen, typename Mapper> class Mapped;

//! Returns a generator which always generates the same value.
template<typename T>
Constant<T> constant(T value);

//! Uses another generator to generate values satisfying a given condition.
//!
//! @param gen   The underlying generator to use.
//! @param pred  The predicate that the generated values must satisfy
template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(Generator gen, Predicate pred);

//! Convenience wrapper for `suchThat(arbitrary<T>, pred)`.
template<typename T, typename Predicate>
SuchThat<Arbitrary<T>, Predicate> suchThat(Predicate pred);

//! Maps a generator of one type to a generator of another type using the given
//! mapping callable.
template<typename Gen, typename Mapper>
Mapped<Gen, Mapper> map(Gen generator, Mapper mapper);

} // namespace gen
} // namespace rc

#include "Common.hpp"
