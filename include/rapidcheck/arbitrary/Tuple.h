#pragma once

namespace rc {

template<typename ...Types> class Arbitrary<std::tuple<Types...>>;
template<typename T1, typename T2> class Arbitrary<std::pair<T1, T2>>;

} // namespace rc

#include "Tuple.hpp"
