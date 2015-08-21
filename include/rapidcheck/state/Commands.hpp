#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/state/gen/ParallelCommands.hpp"
#include "rapidcheck/Assertions.h"
#include <thread>
#include <mutex>
#include "rapidcheck/detail/Barrier.h"

namespace rc {
namespace state {
namespace detail {

template <typename Model, typename Cmd>
void allInterleavingsAreValid(const Commands<Cmd> &left,
                              const Commands<Cmd> &right,
                              int leftIndex,
                              int rightIndex,
                              const Model &state) {
  const auto hasLeft = leftIndex < left.size();
  const auto hasRight = rightIndex < right.size();

  if (hasLeft) {
    auto preState = state;
    auto currentState = state;
    auto command = left[leftIndex];
    command->apply(currentState);
    allInterleavingsAreValid(
        left, right, leftIndex + 1, rightIndex, currentState);
  }

  if (hasRight) {
    auto preState = state;
    auto currentState = state;
    auto command = right[rightIndex];
    command->apply(currentState);
    allInterleavingsAreValid(
        left, right, leftIndex, rightIndex + 1, currentState);
  }
}

template <typename Model, typename Cmd>
void isValidSequence(
  const gen::ParallelCommands<Cmd> &cmds,
  const Model &state) {

  auto currentState = state;

  // Check prefix
  for (const auto &command : cmds.prefix) {
    command->apply(currentState);
  }

  // verify parallel sequences
  allInterleavingsAreValid(
    cmds.left, cmds.right, 0, 0, currentState);
}


template <typename Model, typename Cmd>
struct CommandResult
{
  CommandResult(const std::shared_ptr<const Cmd>& command,
                std::function<void(const Model&)>&& verifyFunc)
    : command(command)
    , verifyFunc(std::move(verifyFunc))
  {
  }

  std::shared_ptr<const Cmd> command;
  std::function<void(const Model&)> verifyFunc;
};

template <typename Model, typename Cmd>
struct ParallelExecutionResult {
  std::vector<CommandResult<Model, Cmd>> prefix;
  std::vector<CommandResult<Model, Cmd>> left;
  std::vector<CommandResult<Model, Cmd>> right;
};


template <typename Model, typename Cmd>
bool hasValidInterleaving(const std::vector<CommandResult<Model, Cmd>> &left,
                          const std::vector<CommandResult<Model, Cmd>> &right,
                          int leftIndex,
                          int rightIndex,
                          const Model &state) {
  const auto hasLeft = leftIndex < left.size();
  const auto hasRight = rightIndex < right.size();

  // Base case, no more commands to verify
  if (!hasRight && !hasLeft) {
    return true;
  }

  auto isValidLeft = false;
  auto isValidRight = false;

  if (hasLeft) {
    try {
      auto preState = state;
      auto currentState = state;
      auto commandResult = left[leftIndex];
      commandResult.command->apply(currentState);
      commandResult.verifyFunc(preState);
      isValidLeft = hasValidInterleaving(
          left, right, leftIndex + 1, rightIndex, currentState);
    } catch (...) {
      // Failed
    }
  }

  if (hasRight) {
    try {
      auto preState = state;
      auto currentState = state;
      auto commandResult = right[rightIndex];
      commandResult.command->apply(currentState);
      commandResult.verifyFunc(preState);
      isValidRight = hasValidInterleaving(
          left, right, leftIndex, rightIndex + 1, currentState);
    } catch (...) {
      // Failed
    }
  }

  return isValidLeft || isValidRight;
}


template <typename Model, typename Cmd>
bool isValidExecution(
   const ParallelExecutionResult<Model, Cmd> &executionResult,
   const Model &state) {
  auto currentState = state;
  // Verify prefix
  for (const auto &commandResult : executionResult.prefix) {
    try {
      auto preState = currentState;
      commandResult.command->apply(currentState);
      commandResult.verifyFunc(preState);
    } catch (...) {
      return false;
    }
 }

 // verify parallel sequences
 return hasValidInterleaving(
     executionResult.left, executionResult.right, 0, 0, currentState);
}

} // detail

template <typename Cmds, typename Model>
void applyAll(const Cmds &commands, Model &state) {
  for (const auto &command : commands) {
    command->apply(state);
  }
}

template <typename Cmds, typename Model, typename Sut>
void runAll(const Cmds &commands, const Model &state, Sut &sut) {
  Model currentState = state;
  for (const auto &command : commands) {
    auto preState = currentState;
    // We need to apply first so we trigger any precondition assertions
    // before running
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


template <typename Cmd>
void showValue(const gen::ParallelCommands<Cmd> &sequence, std::ostream &os) {
  os << "Sequential sequence:" << std::endl;
  show(sequence.prefix, os);
  os << "First parallel sequence:" << std::endl;
  show(sequence.left, os);
  os << "Second parallel sequence:" << std::endl;
  show(sequence.right, os);
}

template <typename Cmds, typename Model, typename Sut>
void runAllParallel(const Cmds &commands, const Model &state, Sut &sut) {
  using Cmd = typename Cmds::Cmd;

  // Verify pre-conditions for all possible interleavings
  detail::isValidSequence(commands, state);

  detail::ParallelExecutionResult<Model, Cmd> result;

  // Run serial commands
  for (const auto &command : commands.prefix) {
    auto verifyFunc = command->run(sut);
    result.prefix.emplace_back(
        detail::CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
  }

  rc::detail::Barrier b(2);

  // Run the two parallel command sequences in separate threads
  auto parallelCommandSeq1 = commands.left;
  auto t1 = std::thread([&parallelCommandSeq1, &result, &sut, &b] {
    b.wait();
    for (const auto &command : parallelCommandSeq1) {
      auto verifyFunc = command->run(sut);
      std::this_thread::yield();
      result.left.emplace_back(
          detail::CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
    }
  });

  auto parallelCommandSeq2 = commands.right;
  auto t2 = std::thread([&parallelCommandSeq2, &result, &sut, &b] {
    b.wait();
    for (const auto &command : parallelCommandSeq2) {
      auto verifyFunc = command->run(sut);
      std::this_thread::yield();
      result.right.emplace_back(
          detail::CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
    }
  });

  t1.join();
  t2.join();

  // Verify that the interleaving can be explained by the model
  if (!isValidExecution(result, state)) {
    RC_FAIL("No valid sequence found");
  }
}

} // namespace state
} // namespace rc
