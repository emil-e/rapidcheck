#pragma once

#include "rapidcheck/detail/ApplyTuple.h"

namespace rc {
namespace gen {

template <typename T, typename... Args>
Gen<T> construct(Gen<Args>... gens) {
  return gen::map(gen::tuple(std::move(gens)...),
                  [](std::tuple<Args...> &&argsTuple) {
                    return rc::detail::applyTuple(
                        std::move(argsTuple),
                        [](Args &&... args) { return T(std::move(args)...); });
                  });
}

template <typename T, typename Arg, typename... Args>
Gen<T> construct() {
  return gen::construct<T>(gen::arbitrary<Arg>(), gen::arbitrary<Args>()...);
}

} // namespace gen
} // namespace rc
