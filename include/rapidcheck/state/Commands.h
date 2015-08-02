#pragma once

#include "rapidcheck/state/Command.h"
#include "rapidcheck/Gen.h"

namespace rc {
namespace state {

/// Alias for a vector of `shared_ptr`s to commands of a particular type. It is
/// a pointer to a const command since commands are treated as values and are
/// thus immutable.
template <typename CommandType>
using Commands = std::vector<std::shared_ptr<const CommandType>>;

/// Applies each command in `commands` to the given state. `commands` is assumed
/// to be a container supporting `begin` and `end` containing pointers to
/// commands appropriate for the given state.
template <typename Cmds, typename Model>
void applyAll(const Cmds &commands, Model &state);

/// Runs each command in `command` on the given system under test assuming the
/// given state. `commands` is assumed to be a container supporting `begin` and
/// `end` containing pointers to commands appropriate for the given state and
/// system under test.
template <typename Cmds, typename Model, typename Sut>
void runAll(const Cmds &commands, const Model &state, Sut &sut);

/// Checks whether command is valid for the given state.
template <typename Cmds, typename Model>
bool isValidSequence(const Cmds &commands, const Model &s0);

} // namespace state
} // namespace rc

#include "Commands.hpp"
