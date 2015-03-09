#pragma once

namespace rc {
namespace newgen {

//! Returns a generator based on the given generator but mapped with the given
//! mapping function.
template<typename T, typename Mapper>
Gen<typename std::result_of<Mapper(T)>::type> map(Mapper &&mapper, Gen<T> gen);

} // namespace newgen
} // namespace rc

#include "Transform.hpp"
