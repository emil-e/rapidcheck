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

template <typename Cmds>
std::vector<Cmds> commandSequencePermutations(
  const gen::detail::ParallelCommandSequence<Cmds>& parallelCmds)
{
   auto& p1 = parallelCmds.parallelCmdSeq1;
   auto& p2 = parallelCmds.parallelCmdSeq2;
   auto indeciesPermutations = commandIndeciesPermutations(p1.size(), p2.size());
   std::vector<Cmds> permutations;

   for (auto& indexVector: indeciesPermutations){
      Cmds cmds = parallelCmds.serialCmdSeq;
      auto cmdSeq = indexVectorToCommandSequence(indexVector, p1, p2);
      cmds.insert(cmds.end(), cmdSeq.begin(), cmdSeq.end());
      permutations.push_back(cmds);
   }

   return permutations;
}

template<typename Cmds>
gen::detail::ParallelCommandSequence<Cmds> splitCommands(const Cmds& commands)
{
  // TODO: Refactor to a single case.
  auto cmdCount = commands.size();
  // If there are fewer than 12 commands, make all parallel
  if (cmdCount < 12) {
    auto groupSize = cmdCount / 2;
    auto p1Begin = commands.begin();
    auto p2Begin = p1Begin + groupSize;
    return gen::detail::ParallelCommandSequence<Cmds>(
          Cmds(),
          Cmds(p1Begin, p1Begin + groupSize),
          Cmds(p2Begin, commands.end()));
  }
  else {
    // If there are more than 12 commands, make the 12 last ones parallel
    auto sBegin  = commands.begin();
    auto p1Begin = sBegin  + cmdCount - 12;
    auto p2Begin = p1Begin + 6;
    return gen::detail::ParallelCommandSequence<Cmds>(
          Cmds(sBegin, p1Begin),
          Cmds(p1Begin, p2Begin),
          Cmds(p2Begin, commands.end()));
  }
}

template <typename Cmds, typename Model>
void verifyCommandSequence(
  const gen::detail::ParallelCommandSequence<Cmds>& parallelCmdSeq,
  const Model &state)
{
  auto possibleSequences = commandSequencePermutations(parallelCmdSeq);
  bool atLeastOneFailed = false;

  for (const auto& seq: possibleSequences) {
    Model currentState = state;
    bool seqFailed = false;
    for (const auto& command: seq) {
      try {
         auto preState = currentState;
         command->apply(currentState);
         command->verify(preState);
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

template <typename Cmds, typename Sut>
void runParallelCmdSeq(const gen::detail::ParallelCommandSequence<Cmds>& parallelCmdSeq, Sut &sut) {
  // Run serial commands
  for (const auto &command : parallelCmdSeq.serialCmdSeq) {
    command->run(sut);
  }

  Barrier b(2);

  // Run the two parallel command sequences in separate threads
  auto& parallelCommandSeq1 = parallelCmdSeq.parallelCmdSeq1;
  auto t1 = std::thread([&parallelCommandSeq1, &sut, &b]{
    b.wait();
    for (const auto &command : parallelCommandSeq1) {
      command->run(sut);
      std::this_thread::yield();
    }
  });

  auto& parallelCommandSeq2 = parallelCmdSeq.parallelCmdSeq2;
  auto t2 = std::thread([&parallelCommandSeq2, &sut, &b]{
    b.wait();
    for (const auto &command : parallelCommandSeq2) {
      command->run(sut);
      std::this_thread::yield();
    }
  });

  t1.join();
  t2.join();
}

template <typename Cmds, typename Model, typename Sut>
void runAllParallel(const Cmds &commands, const Model &state, Sut &sut) {
  // TODO: Verify pre-conditions for all possible interleavings
//  auto parallelCommandSeq = gen::detail::toParallelSequence(commands);
  runParallelCmdSeq(commands, sut);
  // Verify that the interleaving can be explained by the model
  verifyCommandSequence(commands, state);
}

template <typename Cmds, typename Model, typename Sut>
void runAll(const Cmds &commands, const Model &state, Sut &sut) {
  Model currentState = state;
  for (const auto &command : commands) {
    auto preState = currentState;
    // We need to apply first so we trigger any precondition assertions
    // before running
    command->apply(currentState);
    command->run(sut);
    command->verify(preState);
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
