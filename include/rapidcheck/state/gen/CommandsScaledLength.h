#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace gen {

/// Generates a valid commands sequence for the given state initial state
/// consisting of commands of the given type.
/// Uses the scaling factor to scale the length of the command sequence
/// by the provided value
template <typename Cmd, typename GenerationFunc>
Gen<Commands<Cmd>> commandsScaledLength(const typename Cmd::Model &initialState,
                                        double scale, GenerationFunc &&genFunc);

}  // namespace gen
}  // namespace state
}  // namespace rc

#include "CommandsScaledLength.hpp"
