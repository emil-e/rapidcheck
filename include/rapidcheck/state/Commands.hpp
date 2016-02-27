#pragma once

#include "rapidcheck/detail/Results.h"

namespace rc {
namespace state {

template <typename Cmds, typename Model>
void applyAll(const Cmds &commands, Model &state) {
  for (const auto &command : commands) {
    command->preconditions(state);
    command->apply(state);
  }
}

template <typename Cmds, typename Model, typename Sut>
void runAll(const Cmds &commands, const Model &state, Sut &sut) {
  Model currentState = state;
  for (const auto &command : commands) {
    auto preState = currentState;
    command->preconditions(currentState);
    command->apply(currentState);
    command->run(preState, sut);
  }
}

template <typename Cmds, typename Model>
bool isValidSequence(const Cmds &commands, const Model &s0) {
  try {
    auto s1 = s0;
    applyAll(commands, s1);
  } catch (const ::rc::detail::CaseResult &result) {
    if (result.type == ::rc::detail::CaseResult::Type::Discard) {
      return false;
    }
    throw;
  }

  return true;
}

template <typename Model, typename Sut>
void showValue(const std::vector<
                   std::shared_ptr<const state::Command<Model, Sut>>> &commands,
               std::ostream &os) {
  for (const auto &command : commands) {
    command->show(os);
    os << std::endl;
  }
}

} // namespace state
} // namespace rc
