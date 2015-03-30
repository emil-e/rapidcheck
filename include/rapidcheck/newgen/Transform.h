#pragma once

#include "rapidcheck/Gen.h"

namespace rc {
namespace newgen {

//! Returns a generator based on the given generator but mapped with the given
//! mapping function.
template<typename T, typename Mapper>
Gen<typename std::result_of<Mapper(T)>::type> map(Mapper &&mapper, Gen<T> gen);

//! Convenience function which calls `map(Predicate, Gen<T>)` with
//! `newgen::arbitrary<T>`
template<typename T, typename Mapper>
Gen<typename std::result_of<Mapper(T)>::type> map(Mapper &&mapper);

//! Returns a generator that casts the generated values to `T` using
//! `static_cast<T>(...)`.
template<typename T, typename U>
Gen<T> cast(Gen<U> gen);

} // namespace newgen
} // namespace rc

#include "Transform.hpp"
