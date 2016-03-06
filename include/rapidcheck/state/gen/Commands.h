#pragma once

#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace gen {

/// Generates a valid commands sequence for the given state initial state
/// consisting of commands of the given type.
template <typename Model, typename GenerationFunc>
auto commands(const Model &initialState, GenerationFunc &&genFunc)
    -> Gen<Commands<Decay<typename decltype(
        genFunc(initialState))::ValueType::element_type::CommandType>>>;

/// Generates a valid commands sequence for the state returned by the given
/// callable consisting of commands of the given type.
template <typename MakeInitialState, typename GenerationFunc>
auto commands(MakeInitialState &&initialState, GenerationFunc &&genFunc)
    -> Gen<Commands<Decay<typename decltype(
        genFunc(initialState()))::ValueType::element_type::CommandType>>>;

} // namespace gen
} // namespace state
} // namespace rc

#include "Commands.hpp"
