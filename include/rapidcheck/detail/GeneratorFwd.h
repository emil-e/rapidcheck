#pragma once

#include <memory>

namespace rc {
namespace gen {

template<typename T> class Generator;

//! \c std::unique_ptr to \c Generator<T>.
template<typename T>
using GeneratorUP = std::unique_ptr<Generator<T>>;

} // namespace gen
} // namespace rc
