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
void verifyInterleavings(const Commands<Cmd> &left,
                              const Commands<Cmd> &right,
                              int leftIndex,
                              int rightIndex,
                              const Model &state) {
  const auto hasLeft = leftIndex < left.size();
  const auto hasRight = rightIndex < right.size();

  if (hasLeft) {
    auto currentState = state;
    const auto& command = left[leftIndex];
    command->apply(currentState);
    verifyInterleavings(
        left, right, leftIndex + 1, rightIndex, currentState);
  }

  if (hasRight) {
    auto currentState = state;
    const auto& command = right[rightIndex];
    command->apply(currentState);
    verifyInterleavings(
        left, right, leftIndex, rightIndex + 1, currentState);
  }
}

template <typename Model, typename Cmd>
void verifyPreconditions(
  const ParallelCommands<Cmd> &cmds,
  const Model &state) {

  auto currentState = state;

  // Check prefix
  applyAll(cmds.prefix, currentState);

  // verify parallel sequences
  verifyInterleavings(
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

  if (hasLeft) {
    try {
      auto preState = state;
      auto currentState = state;
      auto commandResult = left[leftIndex];
      commandResult.command->apply(currentState);
      commandResult.verifyFunc(preState);
      if (hasValidInterleaving(
              left, right, leftIndex + 1, rightIndex, currentState)) {
        return true;
      }
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
      if (hasValidInterleaving(
              left, right, leftIndex, rightIndex + 1, currentState)) {
        return true;
      }
    } catch (...) {
      // Failed
    }
  }

  return false;
}

template <typename Model, typename Cmd>
void verifyExecution(
    const ParallelExecutionResult<Model, Cmd> &executionResult,
    const Model &state) {
  auto currentState = state;
  // Verify prefix.
  for (const auto &commandResult : executionResult.prefix) {
    auto preState = currentState;
    commandResult.command->apply(currentState);
    commandResult.verifyFunc(preState);
  }

  // verify parallel sequences
  if (!hasValidInterleaving(
        executionResult.left, executionResult.right, 0, 0, currentState)) {
    RC_FAIL("No possible interleaving");
  }
}

} // detail

template <typename Cmds, typename Model>
void applyAll(const Cmds &commands, Model &state) {
  for (const auto &command : commands) {
    command->apply(state);
  }
}

template <typename Cmd, typename Model, typename Sut>
void runAll(const Commands<Cmd> &commands, const Model &state, Sut &sut) {
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

template <typename CommandType>
struct ParallelCommands {
  using Cmd = CommandType;
  using Cmds = Commands<CommandType>;

  ParallelCommands() {}

  ParallelCommands(const Cmds &prefix, const Cmds &left, const Cmds &right)
      : prefix(prefix)
      , left(left)
      , right(right) {}

  Cmds prefix;
  Cmds left;
  Cmds right;
};

template <typename Cmd, typename Model, typename Sut>
std::thread executeCommandSequenceInNewThread(
    const Commands<Cmd> &commands,
    std::vector<detail::CommandResult<Model, Cmd>> &results,
    Sut &sut,
    rc::detail::Barrier &barrier,
    std::exception_ptr &error) {

  return std::thread([&commands, &results, &sut, &barrier, &error] {
    try {
      barrier.wait();
      for (const auto &command : commands) {
        auto verifyFunc = command->run(sut);
        results.emplace_back(
            detail::CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
      }
    } catch (...) {
      error = std::current_exception();
    }
  });
}

template <typename Cmd, typename Model, typename Sut>
void runAllParallel(const ParallelCommands<Cmd> &commands,
                    const Model &state,
                    Sut &sut) {

  // Verify pre-conditions for all possible interleavings
  detail::verifyPreconditions(commands, state);

  detail::ParallelExecutionResult<Model, Cmd> result;

  // Run serial commands
  for (const auto &command : commands.prefix) {
    auto verifyFunc = command->run(sut);
    result.prefix.emplace_back(
        detail::CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
  }

  rc::detail::Barrier b(2);
  std::exception_ptr e1 = nullptr;
  std::exception_ptr e2 = nullptr;

  // Run the two parallel command sequences in separate threads
  auto t1 =
      executeCommandSequenceInNewThread(commands.left, result.left, sut, b, e1);
  auto t2 = executeCommandSequenceInNewThread(
      commands.right, result.right, sut, b, e2);

  t1.join();
  t2.join();

  if (e1 != nullptr) {
    std::rethrow_exception(e1);
  };
  if (e2 != nullptr) {
    std::rethrow_exception(e2);
  };

  // Verify that the interleaving can be explained by the model
  verifyExecution(result, state);
}

template <typename Cmd>
void showValue(const ParallelCommands<Cmd> &sequence, std::ostream &os) {
  os << "Sequential prefix:" << std::endl;
  show(sequence.prefix, os);
  os << "Left branch:" << std::endl;
  show(sequence.left, os);
  os << "Right branch:" << std::endl;
  show(sequence.right, os);
}

} // namespace state
} // namespace rc
