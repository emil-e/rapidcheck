#pragma once

#include <type_traits>
#include <iostream>

namespace rc {
namespace detail {

#define RC_SFINAE_TRAIT(Name, expression)                                      \
  namespace sfinae {                                                           \
  template <typename T, typename = expression>                                 \
  std::true_type test##Name(const T &);                                        \
  std::false_type test##Name(...);                                             \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  using Name = decltype(sfinae::test##Name(std::declval<T>()));

RC_SFINAE_TRAIT(IsStreamInsertible, decltype(std::cout << std::declval<T>()))

} // namespace detail
} // namespace rc
