#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace gen {

/// Generates a valid commands sequence for the given state initial state
/// consisting of commands of the given type.
template <typename Cmd, typename GenerationFunc>
Gen<Commands<Cmd>> commands(const typename Cmd::State &initialState,
                            GenerationFunc &&genFunc);

} // namespace gen
} // namespace state
} // namespace rc

#include "Commands.hpp"
