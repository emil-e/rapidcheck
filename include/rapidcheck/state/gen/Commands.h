#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace gen {

/// Generates a valid commands sequence for the given state initial state
/// consisting of commands of the given type.
template <typename Cmd, typename GenerationFunc>
Gen<Commands<Cmd>> commands(const typename Cmd::Model &initialState,
                            GenerationFunc &&genFunc);

/// Generates a valid commands sequence for the state returned by the given
/// callable consisting of commands of the given type.
template <typename Cmd,
          typename MakeInitialState,
          typename GenerationFunc,
          typename = typename std::enable_if<
              !std::is_same<Decay<MakeInitialState>,
                            typename Cmd::Model>::value>::type>
Gen<Commands<Cmd>> commands(MakeInitialState &&initialState,
                            GenerationFunc &&genFunc);

} // namespace gen
} // namespace state
} // namespace rc

#include "Commands.hpp"
