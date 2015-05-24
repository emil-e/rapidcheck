#pragma once

namespace rc {
namespace state {
namespace detail {

template <typename Cmd>
void Commands<Cmd>::nextState(State &state) const {
  for (const auto &command : commands) {
    command->nextState(state);
  }
}

template <typename Cmd>
void Commands<Cmd>::run(const State &state, Sut &sut) const {
  State currentState = state;
  for (const auto &command : commands) {
    auto preState = currentState;
    // We need to run this first so we trigger any precondition assertions
    // before running
    command->nextState(currentState);
    command->run(preState, sut);
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

template <typename Cmd>
struct ShowType<rc::state::detail::Commands<Cmd>> {
  static void showType(std::ostream &os) {
    os << "Commands<";
    ::rc::detail::showType<Cmd>(os);
    os << ">";
  }
};

} // namespace rc
