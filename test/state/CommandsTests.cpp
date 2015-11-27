#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/StringVec.h"

using namespace rc;
using namespace rc::test;

namespace {

Gen<std::vector<StringVecCmdSP>> pushBackCommands() {
  return gen::container<std::vector<StringVecCmdSP>>(gen::exec(
      []() -> StringVecCmdSP { return std::make_shared<PushBack>(); }));
}

Gen<std::vector<StringVecParCmdSP>> pushBackParCommands() {
  return gen::container<std::vector<StringVecParCmdSP>>(gen::exec(
      []() -> StringVecParCmdSP { return std::make_shared<PushBackPar>(); }));
}

using StringVecParCmds = state::ParallelCommands<StringVecCmd>;

template <typename It>
bool isInterleaved(const It &interleavedBegin,
                   const It &interleavedEnd,
                   const It &source1Begin,
                   const It &source1End,
                   const It &source2Begin,
                   const It &source2End) {
  // Terminate at end of `interleaved`
  if (interleavedBegin == interleavedEnd) {
    return (source1Begin == source1End) && (source2Begin == source2End);
  }

  // Check whether front element of `interleaved` and either source match
  // and remainders are interleaved
  return (source1Begin != source1End && *interleavedBegin == *source1Begin &&
          isInterleaved(interleavedBegin + 1,
                        interleavedEnd,
                        source1Begin + 1,
                        source1End,
                        source2Begin,
                        source2End)) ||
      (source2Begin != source2End && *interleavedBegin == *source2Begin &&
       isInterleaved(interleavedBegin + 1,
                     interleavedEnd,
                     source1Begin,
                     source1End,
                     source2Begin + 1,
                     source2End));
}

} // namespace

TEST_CASE("state::applyAll") {
  prop("returns next state by applying the commands in sequence",
       [](const StringVec &s0) {
         const auto cmds = *pushBackCommands();

         auto expected = s0;
         for (const auto &cmd : cmds) {
           std::ostringstream os;
           cmd->show(os);
           expected.push_back(os.str());
         }

         auto s1 = s0;
         applyAll(cmds, s1);
         RC_ASSERT(s1 == expected);
       });
}

TEST_CASE("state::runAll") {
  prop("runs the commands in sequence",
       [](const StringVec &s0) {
         const auto cmds = *pushBackCommands();

         auto expected = s0;
         for (const auto &cmd : cmds) {
           std::ostringstream os;
           cmd->show(os);
           expected.push_back(os.str());
         }

         StringVec actual(s0);
         runAll(cmds, s0, actual);
         RC_ASSERT(actual == expected);
       });
}

TEST_CASE("state::isValidSequence") {
  prop("returns true if all commands are valid",
       [](StringVec s0) {
         auto sequence = *gen::container<state::Commands<StringVecCmd>>(
                             gen::makeShared<StringVecCmd>());
         RC_ASSERT(isValidSequence(sequence, s0));
       });

  prop("returns false if there is an invalid command in the sequence",
       [](StringVec s0) {
         auto sequence = *gen::container<state::Commands<StringVecCmd>>(
                             gen::makeShared<StringVecCmd>());
         const auto i = *gen::inRange<std::size_t>(0, sequence.size());
         sequence.insert(begin(sequence) + i,
                         std::make_shared<PreNeverHolds>());
         RC_ASSERT(!isValidSequence(sequence, s0));
       });
}

TEST_CASE("state::runAllParallel") {
  prop("runs the commands in sequence",
       [](const StringVec &s0) {
         const auto prefix = *pushBackParCommands();
         const auto left = *pushBackParCommands();
         const auto right = *pushBackParCommands();

         auto expectedPrefix = s0;
         for (const auto &cmd : prefix) {
           std::ostringstream os;
           cmd->show(os);
           expectedPrefix.push_back(os.str());
         }

         StringVec expectedLeft;
         for (const auto &cmd : left) {
           std::ostringstream os;
           cmd->show(os);
           expectedLeft.push_back(os.str());
         }

         StringVec expectedRight;
         for (const auto &cmd : right) {
           std::ostringstream os;
           cmd->show(os);
           expectedRight.push_back(os.str());
         }

         state::ParallelCommands<StringVecParCmd> parallelCmds(
             prefix, left, right);
         auto mutex = std::make_shared<std::mutex>();
         StringVecPar actual(s0, mutex);
         runAllParallel(parallelCmds, s0, actual);

         RC_ASSERT(expectedPrefix.size() <= actual.strings.size());
         RC_ASSERT(std::equal(expectedPrefix.begin(),
                              expectedPrefix.end(),
                              actual.strings.begin()));

         RC_ASSERT(actual.strings.size() ==
                   expectedPrefix.size() + expectedLeft.size() +
                       expectedRight.size());

         RC_ASSERT(isInterleaved(actual.strings.begin() + expectedPrefix.size(),
                                 actual.strings.end(),
                                 expectedLeft.begin(),
                                 expectedLeft.end(),
                                 expectedRight.begin(),
                                 expectedRight.end()));
       });
}

TEST_CASE("show(Commands)") {
  prop("displays each command on a separate line",
       [] {
         const auto cmds = *pushBackCommands();

         std::ostringstream expected;
         for (const auto &cmd : cmds) {
           cmd->show(expected);
           expected << std::endl;
         }

         std::ostringstream actual;
         show(cmds, actual);
         RC_ASSERT(expected.str() == actual.str());
       });
}
