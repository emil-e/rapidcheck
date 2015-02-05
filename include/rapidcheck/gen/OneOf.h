#pragma once

namespace rc {
namespace gen {

template<typename ...Gens> class OneOf;

//! Generates a value by randomly using one of the given generators. All the
//! generators must have the same result type.
template<typename ...Gens>
OneOf<Gens...> oneOf(Gens... generators);

} // namespace gen
} // namespace rc

#include "OneOf.hpp"
