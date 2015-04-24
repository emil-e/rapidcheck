#pragma once

#include "rapidcheck/gen/Arbitrary.h"
#include "rapidcheck/gen/Numeric.h"

namespace rc {

/// Useful utility class for testing that `Arbitrary` is actually used. The key
/// here is that if the default constructor is used, the value will be undefined
/// but if `Arbitrary` is used, the value will be `X`. An extra member is
/// included so that multiple unique values of the same type can be generated.
struct Predictable {
  static constexpr int predictableValue = 1337;
  int value; // TODO provide some default value here?
  int extra;
};

/// Non-copyable version of `Predictable`.
struct NonCopyable : public Predictable {
  NonCopyable() = default;
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
  NonCopyable(NonCopyable &&other) noexcept {
    value = other.value;
    extra = other.extra;
  }

  NonCopyable &operator=(NonCopyable &&other) noexcept {
    value = other.value;
    extra = other.extra;
    return *this;
  }
};

// TODO source file!

static inline bool operator==(const Predictable &lhs, const Predictable &rhs) {
  return (lhs.value == rhs.value) && (lhs.extra == rhs.extra);
}

static inline bool operator<(const Predictable &lhs, const Predictable &rhs) {
  if (lhs.value == rhs.value)
    return lhs.extra < rhs.extra;

  return lhs.value < rhs.value;
}

static inline bool operator>(const Predictable &lhs, const Predictable &rhs) {
  if (lhs.value == rhs.value)
    return lhs.extra > rhs.extra;

  return lhs.value > rhs.value;
}

static inline bool operator<=(const Predictable &lhs, const Predictable &rhs) {
  return (lhs < rhs) || (lhs == rhs);
}

static inline bool operator>=(const Predictable &lhs, const Predictable &rhs) {
  return (lhs > rhs) || (lhs == rhs);
}

static inline void show(const Predictable &value, std::ostream &os) {
  show(value.value, os);
  os << " (" << value.extra << ")";
}

static inline void show(const NonCopyable &value, std::ostream &os) {
  show(value.value, os);
  os << " (" << value.extra << ")";
}

template <>
struct Arbitrary<Predictable> {
  static Gen<Predictable> arbitrary() {
    return gen::map(gen::arbitrary<int>(),
                    [](int extra) {
                      Predictable predictable;
                      predictable.value = Predictable::predictableValue;
                      predictable.extra = extra;
                      return predictable;
                    });
  }
};

template <>
struct Arbitrary<NonCopyable> {
  static Gen<NonCopyable> arbitrary() {
    return gen::map(gen::arbitrary<int>(),
                    [](int extra) {
                      NonCopyable predictable;
                      predictable.value = Predictable::predictableValue;
                      predictable.extra = extra;
                      return predictable;
                    });
  }
};

// These overloads are useful to test if a value was generated using the
// appropriate `Arbitrary` specialization.

static inline bool isArbitraryPredictable(const Predictable &value) {
  return value.value == Predictable::predictableValue;
}

static inline bool
isArbitraryPredictable(const std::pair<const Predictable, Predictable> &value) {
  return isArbitraryPredictable(value.first) &&
      isArbitraryPredictable(value.second);
}

} // namespace rc

namespace std {

template <>
struct hash<rc::Predictable> {
  using argument_type = rc::Predictable;
  using value_type = std::size_t;

  value_type operator()(const rc::Predictable &value) const {
    return std::hash<int>()(value.value) ^ std::hash<int>()(value.extra);
  }
};

template <>
struct hash<rc::NonCopyable> {
  using argument_type = rc::NonCopyable;
  using value_type = std::size_t;

  value_type operator()(const rc::NonCopyable &value) const {
    return std::hash<rc::Predictable>()(value);
  }
};
}
