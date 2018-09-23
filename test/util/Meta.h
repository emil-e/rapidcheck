#pragma once

namespace rc {
namespace test {

/// Instantiates and executes the static member function `exec()` in the type
/// `MetaFunction` once for each type in `Types`. Useful for writing generic
/// tests.
template <typename MetaFunction, typename... Types>
void forEachType() {
  [[maybe_unused]] auto dummy = {(MetaFunction::template exec<Types>(), 0)...};
}

template <typename T, typename Testable>
void templatedProp(const std::string &description, Testable testable) {
  prop(description + " (" + detail::typeToString<T>() + ")", testable);
}

#define TEMPLATED_SECTION(tparam, description)                                 \
  SECTION(std::string(description) + " (" + detail::typeToString<tparam>() +   \
          ")")

} // namespace test
} // namespace rc
