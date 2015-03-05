#pragma once

#include "rapidcheck/Shrinkable.h"

namespace rc {
namespace shrinkable {

//! Returns true if the given predicate returns true for every shrinkable in
//! the entire tree of the given shrinkable. The tree is searched depth-first.
template<typename T, typename Predicate>
bool all(const Shrinkable<T> &shrinkable, Predicate predicate);

} // namespace shrinkable
} // namespace rc

#include "Operations.hpp"
