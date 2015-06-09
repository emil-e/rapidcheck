#pragma once

#include "rapidcheck/state/Command.h"

namespace rc {
namespace state {

/// Alias for a vector of `shared_ptr`s to commands of a particular type. It is
/// a pointer to a const command since commands are treated as values and are
/// thus immutable.
template <typename Cmd>
using Commands = std::vector<std::shared_ptr<const Cmd>>;

/// Applies each command in `commands` to the given state. `commands` is assumed
/// to be a container supporting `begin` and `end` containing pointers to
/// commands appropriate for the given state.
template <typename Cmds, typename State>
void applyAll(const Cmds &commands, State &state);

/// Runs each command in `command` on the given system under test assuming the
/// given state. `commands` is assumed to be a container supporting `begin` and
/// `end` containing pointers to commands appropriate for the given state and
/// system under test.
template <typename Cmds, typename State, typename Sut>
void runAll(const Cmds &commands, const State &state, Sut &sut);

/// Checks whether command is valid for the given state.
template <typename Cmds, typename State>
bool isValidSequence(const Cmds &commands, const State &s0);

} // namespace state
} // namespace rc

#include "Commands.hpp"
