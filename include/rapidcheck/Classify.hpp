#pragma once

#include <string>
#include <initializer_list>

namespace rc {
namespace detail {

struct Stringified {
  template <typename T>
  Stringified(const T &value)
      : str(toString(value)) {}
  std::string str;
};

void tag(std::initializer_list<Stringified> tags);
void classify(std::string condition, std::initializer_list<Stringified> tags);

} // namespace detail
} // namespace rc
