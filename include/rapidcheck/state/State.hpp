#pragma once

#include <algorithm>
#include <cassert>

#include "rapidcheck/state/gen/Commands.h"
#include "rapidcheck/gen/Exec.h"

namespace rc {
namespace state {
namespace detail {

template <typename Cmd,
          typename =
              typename std::is_constructible<Cmd, typename Cmd::State &&>::type>
struct MakeCommand;

template <typename Cmd>
struct MakeCommand<Cmd, std::true_type> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const typename Cmd::State &state) {
    return std::make_shared<Cmd>(state);
  }
};

template <typename Cmd>
struct MakeCommand<Cmd, std::false_type> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const typename Cmd::State &state) {
    return std::make_shared<Cmd>();
  }
};

} // namespace detail

template <typename State, typename Sut, typename GenFunc>
void check(const State &initialState, Sut &sut, GenFunc &&generationFunc) {
  const auto commands =
      *gen::commands<Command<Decay<State>, Sut>>(
          initialState, std::forward<GenFunc>(generationFunc));
  runAll(commands, initialState, sut);
}

template <typename State, typename Sut>
bool isValidCommand(const Command<State, Sut> &command, const State &s0) {
  try {
    auto s1 = s0;
    command.apply(s1);
  } catch (const ::rc::detail::CaseResult &result) {
    if (result.type == ::rc::detail::CaseResult::Type::Discard) {
      return false;
    }
    throw;
  }

  return true;
}

template <typename Cmd, typename... Cmds>
Gen<std::shared_ptr<const typename Cmd::CommandType>>
anyCommand(const typename Cmd::State &state) {
  using CmdSP = std::shared_ptr<const typename Cmd::CommandType>;
  using State = typename Cmd::State;
  using MakeFunc = CmdSP (*)(const State &);
  MakeFunc makeFuncs[] = {&detail::MakeCommand<Cmd>::make,
                          &detail::MakeCommand<Cmds>::make...};

  return [=](const Random &random, int size) {
    auto r = random;
    std::size_t n = r.split().next() % (sizeof...(Cmds) + 1);
    return rc::gen::exec([=] {
      return makeFuncs[n](state);
    })(r, size); // TODO monadic bind
  };
}

} // namespace state
} // namespace rc
