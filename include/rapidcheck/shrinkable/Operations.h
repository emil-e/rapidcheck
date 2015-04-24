#pragma once

#include "rapidcheck/Shrinkable.h"

namespace rc {
namespace shrinkable {

/// Returns true if the given predicate returns true for every shrinkable in
/// the entire tree of the given shrinkable. The tree is searched depth-first.
template <typename T, typename Predicate>
bool all(const Shrinkable<T> &shrinkable, Predicate predicate);

/// Finds a local minimum that satisfies the given predicate. Returns a pair of
/// the minimum value and the number of acceptable values encountered on the way
/// there.
template <typename Predicate, typename T>
std::pair<T, int> findLocalMin(const Shrinkable<T> &shrinkable, Predicate pred);

/// Returns a `Seq` of the immediate shrink values.
template <typename T>
Seq<T> immediateShrinks(const Shrinkable<T> &shrinkable);

} // namespace shrinkable
} // namespace rc

#include "Operations.hpp"
