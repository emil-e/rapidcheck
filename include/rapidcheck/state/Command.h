#pragma once

#include <functional>

namespace rc {
namespace state {

/// Base class for commands used in testing of stateful systems.
///
/// NOTE: Commands are assumed to be immutable so that they can be shared.
template <typename ModelT, typename SutT>
class Command {
public:
  using Model = ModelT;
  using Sut = SutT;
  using CommandType = Command<Model, Sut>;
  using Verify = std::function<void(const Model&)>;

  /// Applies the command to the given model state. Default implementation does
  /// nothing.
  ///
  /// Assert preconditions using `RC_PRE` or `RC_DISCARD`. If preconditions do
  /// not hold, command will be discarded and a new one will be generated.
  virtual void apply(Model &s0) const;

  /// Applies this command to the given system under test assuming it has the
  /// given state. Default implementation does nothing.
  ///
  /// Use rapidcheck assertion macros to check that the system behaves
  /// properly.
  virtual void run(const Model &s0, Sut &sut) const;

  /// Alterantive run function that allows rapidcheck to defer checking of the
  /// system behaviour. This version must be used for parallel testing and can
  /// be used for sequential testing.
  ///
  /// Use rapidcheck assertion macros in the returned function to check that the 
  /// system behaves properly.
  virtual Verify run(Sut &sut) const;

  /// Outputs a human readable representation of the command to the given
  /// output stream.
  virtual void show(std::ostream &os) const;

  /// Convenience method which given a state returns the state after this
  /// command has been applied using `apply`.
  Model nextState(const Model &s0) const;

  virtual ~Command() = default;
};

} // namespace state
} // namespace rc

#include "Command.hpp"
