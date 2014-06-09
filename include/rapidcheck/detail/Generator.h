#pragma once

namespace rc {
namespace detail {

//! Creates a `GeneratorUP` to a copy of the given generator.
template<typename Gen>
GeneratorUP<typename Gen::GeneratedType> makeGeneratorUP(Gen generator);

} // namespace detail
} // namespace rc
