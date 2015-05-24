#pragma once

namespace rc {
namespace state {
namespace detail {

template <typename Cmd>
typename Commands<Cmd>::State
Commands<Cmd>::nextState(const State &state) const {
  State currentState = state;
  for (const auto &command : commands) {
    currentState = command->nextState(currentState);
  }
  return currentState;
}

template <typename Cmd>
void Commands<Cmd>::run(const State &state, Sut &sut) const {
  State currentState = state;
  for (const auto &command : commands) {
    auto nextState = command->nextState(currentState);
    command->run(currentState, sut);
    currentState = nextState;
  }
}

template <typename Cmd>
void Commands<Cmd>::show(std::ostream &os) const {
  for (const auto &command : commands) {
    command->show(os);
    os << std::endl;
  }
}

} // namespace detail
} // namespace state
} // namespace rc
