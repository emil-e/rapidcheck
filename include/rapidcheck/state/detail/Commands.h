#pragma once

#include "rapidcheck/state/Command.h"

namespace rc {
namespace state {
namespace detail {

/// Sequence of commands. Runs the contained commands in sequence.
template <typename Cmd>
struct Commands : public Command<typename Cmd::State, typename Cmd::Sut> {
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using State = typename Cmd::State;
  using Sut = typename Cmd::Sut;

  void nextState(State &state) const override;
  void run(const State &state, Sut &sut) const override;
  void show(std::ostream &os) const override;

  std::vector<CmdSP> commands;
};

} // namespace detail
} // namespace state
} // namespace rc

#include "Commands.hpp"
