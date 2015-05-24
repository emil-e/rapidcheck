#pragma once

#include <algorithm>
#include <cassert>

#include "rapidcheck/state/detail/CommandsGen.h"

namespace rc {
namespace state {
namespace detail {

template <typename Cmd,
          typename =
              typename std::is_constructible<Cmd, typename Cmd::State &&>::type>
struct CommandMaker;

template <typename Cmd>
struct CommandMaker<Cmd, std::true_type> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const typename Cmd::State &state) {
    return std::make_shared<Cmd>(state);
  }
};

template <typename Cmd>
struct CommandMaker<Cmd, std::false_type> {
  static std::shared_ptr<const typename Cmd::CommandType>
  make(const typename Cmd::State &state) {
    return std::make_shared<Cmd>();
  }
};

template <typename... Cmds>
struct CommandPicker;

template <typename Cmd>
struct CommandPicker<Cmd> {
  static std::shared_ptr<const typename Cmd::CommandType>
  pick(const typename Cmd::State &state, int n) {
    return CommandMaker<Cmd>::make(state);
  }
};

template <typename Cmd, typename... Cmds>
struct CommandPicker<Cmd, Cmds...> {
  static std::shared_ptr<const typename Cmd::CommandType>
  pick(const typename Cmd::State &state, int n) {
    return (n == 0) ? CommandMaker<Cmd>::make(state)
                    : CommandPicker<Cmds...>::pick(state, n - 1);
  }
};

} // namespace detail

template <typename State, typename Sut, typename GenFunc>
void check(const State &initialState, Sut &sut, GenFunc &&generationFunc) {
  const auto commands =
      *detail::genCommands<Command<Decay<State>, Sut>>(
          initialState, std::forward<GenFunc>(generationFunc));
  commands.run(initialState, sut);
}

template <typename State, typename Sut>
bool isValidCommand(const Command<State, Sut> &command, const State &s0) {
  try {
    command.nextState(s0);
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
  return [=](const Random &random, int size) {
    auto r = random;
    std::size_t n = r.split().next() % (sizeof...(Cmds) + 1);
    return gen::exec([=] {
      return detail::CommandPicker<Cmd, Cmds...>::pick(state, n);
    })(r, size); // TODO monadic bind
  };
}

} // namespace state
} // namespace rc
