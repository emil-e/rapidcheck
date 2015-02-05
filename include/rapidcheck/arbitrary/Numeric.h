#pragma once

namespace rc {

template<typename T> class Arbitrary;
template<> class Arbitrary<float>;
template<> class Arbitrary<double>;
template<> class Arbitrary<bool>;

} // namespace rc

#include "Numeric.hpp"
