#pragma once

#include "rapidcheck/gen/Container.h"
#include "rapidcheck/gen/Select.h"
#include "rapidcheck/gen/Transform.h"

namespace rc {
namespace gen {
namespace boost {

template <typename... Ts>
Gen<::boost::variant<Ts...>> variant(Gen<Ts>... gens) {
  return rc::gen::oneOf(rc::gen::map(std::move(gens),[](auto&& v){ return ::boost::variant<Ts...>(std::move(v));})...);
}

} // namespace boost
} // namespace gen

template <typename T>
struct Arbitrary<boost::detail::variant::recursive_flag<T>> {
  static Gen<T> arbitrary() {
    return gen::arbitrary<T>();
  }
};

template <typename... Ts>
struct Arbitrary<boost::variant<Ts...>> {
  static Gen<boost::variant<Ts...>> arbitrary() {
    return gen::boost::variant(gen::arbitrary<Ts>()...);
  }
};

template <typename... Ts>
void showValue(const boost::variant<Ts...> &x, std::ostream &os) {
  os << x;
}

} // namespace rc
