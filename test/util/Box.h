#pragma once

namespace rc {
namespace test {

/// Utility for testing showValue(...) implementations to ensure that they are
/// using rc::show(...) for their sub values. Implicit conversion to its string
/// value.
struct Box {
  Box(int x)
      : value(x) {}
  int value;

  std::string str() const { return "|" + std::to_string(value) + "|"; }
};

inline void showValue(const Box &value, std::ostream &os) { os << value.str(); }

} // namespace test

template <>
struct Arbitrary<test::Box> {
  static Gen<test::Box> arbitrary() {
    return gen::cast<test::Box>(gen::arbitrary<int>());
  }
};

} // namespace rc
