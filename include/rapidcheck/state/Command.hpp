#pragma once

#include "rapidcheck/detail/Platform.h"
#include "rapidcheck/detail/ShowType.h"
#include "rapidcheck/detail/Results.h"

namespace rc {
namespace state {

template <typename Model, typename Sut>
void Command<Model, Sut>::apply(Model &s0) const {}

template <typename Model, typename Sut>
void Command<Model, Sut>::run(const Model &s0, Sut &sut) const {
  run(sut)(s0);
}

template <typename Model, typename Sut>
typename Command<Model, Sut>::Verify Command<Model, Sut>::run(Sut &sut) const {
  std::stringstream ss;
  ss << "Command \"";
  this->show(ss);
  ss << "\" does not implement run function";

  throw ::rc::detail::CaseResult(::rc::detail::CaseResult::Type::Failure,
                                 ss.str());
}

template <typename Model, typename Sut>
void Command<Model, Sut>::show(std::ostream &os) const {
  os << ::rc::detail::demangle(typeid(*this).name());
}

template <typename Model, typename Sut>
void showValue(const Command<Model, Sut> &command, std::ostream &os) {
  command.show(os);
}

template <typename Model, typename Sut>
Model Command<Model, Sut>::nextState(const Model &s0) const {
  auto s1 = s0;
  apply(s1);
  return s1;
}

} // namespace state

template <typename Model, typename Sut>
struct ShowType<rc::state::Command<Model, Sut>> {
  static void showType(std::ostream &os) {
    os << "Command<";
    ::rc::detail::showType<Model>(os);
    os << ", ";
    ::rc::detail::showType<Sut>(os);
    os << ">";
  }
};

} // namespace rc
