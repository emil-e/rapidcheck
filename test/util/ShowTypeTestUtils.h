#pragma once

namespace rc {
namespace test {

struct Foo {};
struct Bar {};
struct Baz {};
struct NonspecializedStruct {};
class NonspecializedClass {};
enum NonspecializedEnum {};
enum class NonspecializedEnumClass {};

template <typename T>
struct NonspecializedTemplate {};

} // namespace test

template <>
struct ShowType<test::Foo> {
  static void showType(std::ostream &os) { os << "FFoo"; }
};

template <>
struct ShowType<test::Bar> {
  static void showType(std::ostream &os) { os << "BBar"; }
};

template <>
struct ShowType<test::Baz> {
  static void showType(std::ostream &os) { os << "BBaz"; }
};

} // namespace rc
