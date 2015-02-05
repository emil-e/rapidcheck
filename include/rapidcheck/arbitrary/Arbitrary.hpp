#pragma once

namespace rc {
namespace gen {

template<typename T>
::rc::Arbitrary<T> arbitrary() { return ::rc::Arbitrary<T>(); }

} // namespace gen
} // namespace rc
