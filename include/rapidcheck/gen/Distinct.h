#pragma once

#include "rapidcheck/Traits.h"

namespace rc {
namespace gen {

//! Generates a value using the given generator that is not equal to the given
//! value.
template<typename Generator, typename T>
SuchThat<Decay<Generator>, predicate::Not<predicate::Equals<Decay<T>>>>
distinctFrom(Generator &&generator, T &&value);

//! Generates an arbitrary value using the given generator that is not equal to
//! the given value.
template<typename T>
SuchThat<Arbitrary<Decay<T>>, predicate::Not<predicate::Equals<Decay<T>>>>
distinctFrom(T &&value);

} // namespace gen
} // namespace rc

#include "Distinct.hpp"
