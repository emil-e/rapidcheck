#pragma once

#include "Common.h"
#include "rapidcheck/predicate/Predicates.h"

namespace rc {
namespace gen {

template<typename T> class Ranged;

//! Generates an arbitrary value between \c min and \c max. Both \c min and
//! \c max are included in the range.
//!
//! @param min  The minimum value.
//! @param max  The maximum value.
template<typename T>
Ranged<T> ranged(T min, T max);

//! Generates a non-zero value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
SuchThat<Arbitrary<T>, predicate::Not<predicate::Equals<T>>> nonZero();

//! Generates a positive value (> 0) of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
SuchThat<Arbitrary<T>, predicate::GreaterThan<T>> positive();

//! Generates a negative (< 0) value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
SuchThat<Arbitrary<T>, predicate::LessThan<T>> negative();

//! Generates a non-negative (>= 0) value of type `T`.
//!
//! @tparam T  An integral type.
template<typename T>
SuchThat<Arbitrary<T>, predicate::GreaterEqThan<T>> nonNegative();

} // namespace gen
} // namespace rc

#include "Numeric.hpp"
