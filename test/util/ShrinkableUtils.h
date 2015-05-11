#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/Assertions.h"

namespace rc {
namespace test {

Gen<std::vector<std::size_t>> genPath();

template <typename T>
Maybe<std::pair<T, std::size_t>> atOrLast(Seq<T> seq, std::size_t index) {
  Maybe<T> value;
  std::size_t i = 0;
  while (auto next = seq.next()) {
    if (!next) {
      break;
    }

    if (i == index) {
      value = std::move(next);
      break;
    }

    ++i;
  }

  if (value) {
    return std::make_pair(std::move(*value), i);
  } else {
    return Nothing;
  }
}

/// Calls `assertion` with a value and some shrink of the value when going down
/// arbitrary paths into the shrinkable tree. `RC_ASSERT` in this function to do
/// something useful.
template <typename T, typename Assertion>
void onAnyPath(const Shrinkable<T> &shrinkable, Assertion assertion) {
  Shrinkable<T> current = shrinkable;
  for (const auto n : *genPath()) {
    auto shrink = atOrLast(current.shrinks(), n);
    if (!shrink) {
      return;
    }
    assertion(current, shrink->first);
    current = std::move(shrink->first);
  }
}

/// Checks equivalence by randomly traversing a path through the pair of
/// shrinkables at each step comparing values.
template <typename T>
void assertEquivalent(const Shrinkable<T> &a, const Shrinkable<T> &b) {
  auto currentA = a;
  auto currentB = b;
  RC_ASSERT(currentA.value() == currentB.value());

  for (const auto n : *genPath()) {
    auto shrinkA = atOrLast(currentA.shrinks(), n);
    auto shrinkB = atOrLast(currentB.shrinks(), n);

    if (!shrinkA && !shrinkB) {
      return;
    }

    if ((!shrinkA || !shrinkB) || (shrinkA->second != shrinkB->second)) {
      RC_FAIL("Number of shrinks not equal");
    }

    currentA = std::move(shrinkA->first);
    currentB = std::move(shrinkB->first);
    RC_ASSERT(currentA.value() == currentB.value());
  }
}

/// Returns a `Seq` which counts down from `x - 1` to `0` (inclusive).
Seq<int> countdownSeq(int x);

/// Returns a `Shrinkable` which shrinks by counting down from the given number.
Shrinkable<int> countdownShrinkable(int x);

} // namespace test
} // namespace rc
