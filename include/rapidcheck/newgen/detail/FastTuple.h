#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/detail/FastTuple.h"

// TODO document

namespace rc {
namespace newgen {
namespace detail {

//! Given a number of generators, returns a generator for a tuple with the value
//! types of those generators.
template<typename ...Ts>
Gen<rc::detail::FastTuple<Ts...>> fastTuple(Gen<Ts> ...gens);

} // namespace detail
} // namespace newgen
} // namespace rc

#include "FastTuple.hpp"
