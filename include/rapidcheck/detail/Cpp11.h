#pragma once

#include <boost/lexical_cast/try_lexical_convert.hpp>
#include <boost/math/special_functions/trunc.hpp>

#include <string>

// The android gcc 4.9 toolchain does not support std::to_string. We replicate
// that functionality using boost.

namespace rc {
template <typename T>
std::string to_string(T value) {
  ::std::string res;
  return boost::conversion::try_lexical_convert(value, res) ? res : "";
}

template <typename T>
T trunc(T value) {
  return boost::math::trunc(value);
}
} // namespace rc
