#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace detail {

/// Create a commands generator with the given initial state and command
/// generator factory function.
template <typename Cmd, typename State, typename GenerationFunc>
Gen<Commands<Cmd>> genCommands(State &&initialState, GenerationFunc &&genFunc);

} // namespace detail
} // namespace state
} // namespace rc

#include "CommandsGen.hpp"
