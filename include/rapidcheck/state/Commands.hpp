#pragma once

namespace rc {
namespace state {

template <typename Cmds, typename State>
void applyAll(const Cmds &commands, State &state) {
  for (const auto &command : commands) {
    command->apply(state);
  }
}

template <typename Cmds, typename State, typename Sut>
void runAll(const Cmds &commands, const State &state, Sut &sut) {
  State currentState = state;
  for (const auto &command : commands) {
    auto preState = currentState;
    // We need to apply first so we trigger any precondition assertions
    // before running
    command->apply(currentState);
    command->run(preState, sut);
  }
}

template <typename Cmds, typename State>
bool isValidSequence(const Cmds &commands, const State &s0) {
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

template <typename State, typename Sut>
void showValue(const std::vector<
                   std::shared_ptr<const state::Command<State, Sut>>> &commands,
               std::ostream &os) {
  for (const auto &command : commands) {
    command->show(os);
    os << std::endl;
  }
}

} // namespace state
} // namespace rc
