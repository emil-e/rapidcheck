#pragma once

namespace rc {
namespace state {

template <typename State, typename Sut>
State Command<State, Sut>::nextState(const State &s0) const {
  return s0;
}

template <typename State, typename Sut>
void Command<State, Sut>::run(const State &s0, Sut &sut) const {}

template <typename State, typename Sut>
void Command<State, Sut>::show(std::ostream &os) const {
  os << ::rc::detail::demangle(typeid(*this).name());
}

template <typename State, typename Sut>
void showValue(const Command<State, Sut> &command, std::ostream &os) {
  command.show(os);
}

} // namespace state

template <typename State, typename Sut>
struct ShowType<rc::state::Command<State, Sut>> {
  static void showType(std::ostream &os) {
    os << "Command<";
    ::rc::detail::showType<State>(os);
    os << ", ";
    ::rc::detail::showType<Sut>(os);
    os << ">";
  }
};

} // namespace rc
