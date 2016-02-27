#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/IntVec.h"

using namespace rc;
using namespace rc::test;

namespace {

Gen<std::vector<IntVecCmdSP>> pushBackCommands() {
  return gen::container<std::vector<IntVecCmdSP>>(gen::exec(
      []() -> IntVecCmdSP { return std::make_shared<PushBack>(); }));
}

} // namespace

TEST_CASE("state::applyAll") {
  prop("returns next state by applying the commands in sequence",
       [](const IntVec &s0) {
         const auto cmds = *pushBackCommands();

         auto expected = s0;
         for (const auto &cmd : cmds) {
           expected.push_back(static_cast<const PushBack &>(*cmd).value);
         }

         auto s1 = s0;
         applyAll(cmds, s1);
         RC_ASSERT(s1 == expected);
       });
}

TEST_CASE("state::runAll") {
  prop("runs the commands in sequence",
       [](const IntVec &s0) {
         const auto cmds = *pushBackCommands();

         auto expected = s0;
         for (const auto &cmd : cmds) {
           expected.push_back(static_cast<const PushBack &>(*cmd).value);
         }

         IntVec actual(s0);
         runAll(cmds, s0, actual);
         RC_ASSERT(actual == expected);
       });
}

TEST_CASE("state::isValidSequence") {
  prop("returns true if all commands are valid",
       [](IntVec s0) {
         auto sequence = *gen::container<state::Commands<IntVecCmd>>(
                             gen::makeShared<IntVecCmd>());
         RC_ASSERT(isValidSequence(sequence, s0));
       });

  prop("returns false if there is an invalid command in the sequence",
       [](IntVec s0) {
         auto sequence = *gen::container<state::Commands<IntVecCmd>>(
                             gen::makeShared<IntVecCmd>());
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
