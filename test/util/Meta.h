#pragma once

namespace rc {
namespace meta {

/// Instantiates and executes the static member function `exec()` in the type
/// `MetaFunction` once for each type in `Types`. Useful for writing generic
/// tests.
template <typename MetaFunction, typename... Types>
void forEachType() {
  auto dummy = {(MetaFunction::template exec<Types>(), 0)...};
}

} // namespace meta
} // namespace rc
