#pragma once

#include "rapidcheck/detail/ShowType.h"

namespace rc {

/// So that we in templated tests can compare map pairs with their non-const-key
/// equivalents.
template <typename T1, typename T2>
bool operator==(const std::pair<const T1, T2> &lhs,
                const std::pair<T1, T2> &rhs) {
  return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

/// Returns the size of the given container by counting them through iterators.
template <typename T>
typename T::size_type containerSize(const T &container) {
  return std::distance(begin(container), end(container));
}

/// Returns the set difference between the two given containers as computed by
/// `std::set_difference`.
template <typename T, typename C1, typename C2>
std::vector<T> setDifference(const C1 &c1, const C2 &c2) {
  std::vector<T> cs1(begin(c1), end(c1));
  std::sort(cs1.begin(), cs1.end());
  std::vector<T> cs2(begin(c2), end(c2));
  std::sort(cs2.begin(), cs2.end());
  std::vector<T> result;
  std::set_difference(cs1.begin(),
                      cs1.end(),
                      cs2.begin(),
                      cs2.end(),
                      std::back_inserter(result));
  return result;
}

struct NonComparable {
  NonComparable(const char *x)
      : value(x) {}

  std::string value;
};

} // namespace rc
