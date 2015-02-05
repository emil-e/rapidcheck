#pragma once

#include <limits>
#include <cmath>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace rc {

//! Template for generators of arbitrary values of different types. Specialize
//! this template to provide generation for custom types.
//!
//! @tparam T  The generated type.
template<typename T> class Arbitrary;

namespace gen {

//! Arbitrary generator for type `T`.
//!
//! @tparam T  The generated type.
template<typename T>
::rc::Arbitrary<T> arbitrary();

} // namespace gen
} // namespace rc

#include "Arbitrary.hpp"
