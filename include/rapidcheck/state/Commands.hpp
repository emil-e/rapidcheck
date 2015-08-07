#pragma once

#include "rapidcheck/detail/Results.h"
#include "rapidcheck/state/gen/ParallelCommands.hpp"
#include "rapidcheck/Assertions.h"
#include <thread>
#include <mutex>

namespace rc {
namespace state {

template <typename Cmds, typename Model>
void applyAll(const Cmds &commands, Model &state) {
  for (const auto &command : commands) {
    command->apply(state);
  }
}

class Barrier
{
public:
  Barrier(unsigned int count)
    : m_threshold(count)
    , m_count(count)
    , m_generation(0)
  {
    if (count == 0)
    {
      throw std::invalid_argument("count cannot be zero.");
    }
  }

  bool wait()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    unsigned int gen = m_generation;

    if (--m_count == 0)
    {
      m_generation++;
      m_count = m_threshold;
      m_cond.notify_all();
      return true;
    }

    while (gen == m_generation)
      m_cond.wait(lock);
    return false;
  }

private:
  std::mutex m_mutex;
  std::condition_variable m_cond;
  unsigned int m_threshold;
  unsigned int m_count;
  unsigned int m_generation;
};


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

std::vector<std::vector<int>>
commandIndeciesPermutations(int s1count, int s2count);

template<typename Cmds>
Cmds indexVectorToCommandSequence(std::vector<int> indecies, const Cmds& s1, const Cmds& s2)
{
   Cmds cmds;
   auto s1iterator = s1.begin();
   auto s2iterator = s2.begin();

   for (auto& i: indecies) {
      if (i == 0) {
         cmds.push_back(*s1iterator);
         std::advance(s1iterator, 1);
      }
      else {
         cmds.push_back(*s2iterator);
         std::advance(s2iterator, 1);
      }
   }

   return cmds;
}

template <typename Model, typename Cmd>
std::vector<std::vector<CommandResult<Model, Cmd>>> commandSequencePermutations(
  const ParallelExecutionResult<Model, Cmd>& executionResult)
{
   auto p1 = executionResult.left;
   auto p2 = executionResult.right;
   auto indeciesPermutations = commandIndeciesPermutations(p1.size(), p2.size());
   std::vector<std::vector<CommandResult<Model, Cmd>>> permutations;

   for (auto& indexVector: indeciesPermutations){
      std::vector<CommandResult<Model, Cmd>> cmds = executionResult.prefix;
      auto cmdSeq = indexVectorToCommandSequence(indexVector, p1, p2);
      cmds.insert(cmds.end(), cmdSeq.begin(), cmdSeq.end());
      permutations.push_back(cmds);
   }

   return permutations;
}

template <typename Model, typename Cmd>
void verifyCommandSequence(
  const ParallelExecutionResult<Model, Cmd>& executionResult,
  const Model &state)
{
  auto possibleSequences = commandSequencePermutations(executionResult);
  bool atLeastOneFailed = false;

  for (const auto& seq: possibleSequences) {
    Model currentState = state;
    bool seqFailed = false;
    for (const auto& commandResult: seq) {
      try {
         auto preState = currentState;
         commandResult.command->apply(currentState);
         commandResult.verifyFunc(preState);
      }
      catch (...) {
         atLeastOneFailed = true;
         seqFailed = true;
      }
    }
    if (!seqFailed)
    {
       return; // There was a valid sequence
    }
  }

  if (atLeastOneFailed) {
     RC_FAIL("No valid sequence found");
  }
}

template <typename Model, typename Cmd, typename Sut>
ParallelExecutionResult<Model, Cmd> runParallelCmdSeq(
    const gen::detail::ParallelCommandSequence<Commands<Cmd>> &parallelCmdSeq,
    Sut &sut) {

  ParallelExecutionResult<Model, Cmd> result;

  // Run serial commands
  for (const auto &command : parallelCmdSeq.prefix.value()) {
    auto verifyFunc = command->run(sut);
    result.prefix.emplace_back(
        CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
  }

  Barrier b(2);

  // Run the two parallel command sequences in separate threads
  auto parallelCommandSeq1 = parallelCmdSeq.left.value();
  auto t1 = std::thread([&parallelCommandSeq1, &result, &sut, &b] {
    b.wait();
    for (const auto &command : parallelCommandSeq1) {
      auto verifyFunc = command->run(sut);
      std::this_thread::yield();
      result.left.emplace_back(
          CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
    }
  });

  auto parallelCommandSeq2 = parallelCmdSeq.right.value();
  auto t2 = std::thread([&parallelCommandSeq2, &result, &sut, &b] {
    b.wait();
    for (const auto &command : parallelCommandSeq2) {
      auto verifyFunc = command->run(sut);
      std::this_thread::yield();
      result.right.emplace_back(
        CommandResult<Model, Cmd>(command, std::move(verifyFunc)));
    }
  });

  t1.join();
  t2.join();

  return result;
}

template <typename Cmds, typename Model, typename Sut>
void runAllParallel(const Cmds &commands, const Model &state, Sut &sut) {
  // TODO: Verify pre-conditions for all possible interleavings
  auto result = runParallelCmdSeq<Model>(commands, sut);
  // Verify that the interleaving can be explained by the model
  verifyCommandSequence(result, state);
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

} // namespace state
} // namespace rc
