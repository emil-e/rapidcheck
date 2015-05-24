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

} // namespace state
} // namespace rc
