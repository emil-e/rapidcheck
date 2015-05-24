#pragma once

namespace rc {
namespace state {

/// Base class for commands used in testing of stateful systems.
///
/// NOTE: Commands are assumed to be immutable so that they can be shared.
template <typename StateT, typename SutT>
class Command {
public:
  using State = StateT;
  using Sut = SutT;
  using CommandType = Command<State, Sut>;

  /// Applies the command to the given model state. Default implementation does
  /// nothing.
  ///
  /// Assert preconditions using `RC_PRE` or `RC_DISCARD`. If preconditions do
  /// not hold, command will be discarded and a new one will be generated.
  virtual void nextState(State &s0) const;

  /// Applies this command to the given system under test assuming it has the
  /// given state. Default implementation does nothing.
  ///
  /// Use rapidcheck assertion macros to check that the system behaves
  /// properly.
  virtual void run(const State &s0, Sut &sut) const;

  /// Outputs a human readable representation of the command to the given
  /// output stream.
  virtual void show(std::ostream &os) const;

  virtual ~Command() = default;
};

} // namespace state
} // namespace rc

#include "Command.hpp"
