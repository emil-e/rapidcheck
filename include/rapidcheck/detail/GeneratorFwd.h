#pragma once

namespace rc {
namespace gen {

class ValueDescription;
class UntypedGenerator;
template<typename T> class Generator;

//! \c std::unique_ptr to \c Generator<T>.
template<typename T>
using GeneratorUP = std::unique_ptr<Generator<T>>;

typedef std::unique_ptr<UntypedGenerator> UntypedGeneratorUP;

} // namespace gen
} // namespace rc
