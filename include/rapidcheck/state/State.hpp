#pragma once

#include <algorithm>
#include <cassert>

#include "rapidcheck/state/gen/Commands.h"
#include "rapidcheck/gen/Exec.h"

namespace rc {
namespace state {

template <typename Model, typename Sut, typename GenFunc>
void check(const Model &initialState, Sut &sut, GenFunc &&generationFunc) {
  const auto commands =
      *gen::commands<Command<Model, Sut>>(
          initialState, std::forward<GenFunc>(generationFunc));
  runAll(commands, initialState, sut);
}

template <typename Model, typename Sut, typename GenFunc>
void checkParallel(const Model &initialState, Sut &sut, GenFunc &&generationFunc) {
  const auto commands =
      *gen::parallelCommands<Command<Model, Sut>>(
          initialState, std::forward<GenFunc>(generationFunc));
  runAllParallel(commands, initialState, sut);
}


template <typename Model, typename Sut>
bool isValidCommand(const Command<Model, Sut> &command, const Model &s0) {
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

} // namespace state
} // namespace rc
