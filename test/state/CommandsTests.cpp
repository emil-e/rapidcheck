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

TEST_CASE("state::splitCommands") {
  prop("splits commands into three equally(ish) sized groups",
       [] {
         const auto cmds = *pushBackCommands();
         auto parallelCmdSeq = splitCommands(cmds);

         // Merged sequences should equal original sequence
         auto mergedCmds = parallelCmdSeq.serialCmdSeq;
         mergedCmds.insert(mergedCmds.end(), parallelCmdSeq.parallelCmdSeq1.begin(), parallelCmdSeq.parallelCmdSeq1.end());
         mergedCmds.insert(mergedCmds.end(), parallelCmdSeq.parallelCmdSeq2.begin(), parallelCmdSeq.parallelCmdSeq2.end());

         RC_ASSERT(std::equal(cmds.begin(), cmds.end(), mergedCmds.begin()));
       });
}
