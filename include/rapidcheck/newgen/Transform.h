#pragma once

#include "rapidcheck/Gen.h"

namespace rc {
namespace newgen {

//! Returns a generator based on the given generator but mapped with the given
//! mapping function.
template<typename T, typename Mapper>
Gen<Decay<typename std::result_of<Mapper(T)>::type>>
map(Gen<T> gen, Mapper &&mapper);

//! Convenience function which calls `map(Gen<T>, Mapper)` with
//! `newgen::arbitrary<T>`
template<typename T, typename Mapper>
Gen<Decay<typename std::result_of<Mapper(T)>::type>> map(Mapper &&mapper);

//! Returns a generator that casts the generated values to `T` using
//! `static_cast<T>(...)`.
template<typename T, typename U>
Gen<T> cast(Gen<U> gen);

//! Returns a generator that uses the given generator to generate only values
//! that match the given predicate. Throws a `GenerationFailure` if such a value
//! cannot be generated after an unspecified number of tries.
template<typename T, typename Predicate>
Gen<T> suchThat(Gen<T> gen, Predicate &&pred);

//! Convenience function which calls `suchThat(Gen<T>, Predicate)` with
//! `newgen::arbitrary<T>`
template<typename T, typename Predicate>
Gen<T> suchThat(Predicate &&pred);

} // namespace newgen
} // namespace rc

#include "Transform.hpp"
