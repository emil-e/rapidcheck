#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace gen {

/// Generates a valid parallel command sequence for the given initial state
/// consisting of commands of the given type.
template <typename Cmd, typename GenerationFunc>
Gen<ParallelCommands<Cmd>>
parallelCommands(const typename Cmd::Model &initialState,
                 GenerationFunc &&genFunc);

} // namespace gen
} // namespace state
} // namespace rc

#include "ParallelCommands.hpp"
