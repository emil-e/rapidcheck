#pragma once

#include "rapidcheck/Gen.h"

namespace rc {
namespace newgen {

//! Given a number of generators, returns a generator for a tuple with the value
//! types of those generators.
template<typename ...Ts>
Gen<std::tuple<Ts...>> tuple(Gen<Ts> ...gens);

} // namespace newgen
} // namespace rc

#include "Tuple.hpp"
