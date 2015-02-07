#pragma once

#include "rapidcheck/detail/Traits.h"

namespace rc {
namespace gen {

//! Generates a value using the given generator that is not equal to the given
//! value.
template<typename Generator, typename T>
SuchThat<detail::DecayT<Generator>,
         predicate::Not<predicate::Equals<detail::DecayT<T>>>>
distinctFrom(Generator &&generator, T &&value);

//! Generates an arbitrary value using the given generator that is not equal to
//! the given value.
template<typename T>
SuchThat<Arbitrary<detail::DecayT<T>>,
         predicate::Not<predicate::Equals<detail::DecayT<T>>>>
distinctFrom(T &&value);

} // namespace gen
} // namespace rc

#include "Distinct.hpp"
