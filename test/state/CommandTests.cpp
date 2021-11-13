#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/IntVec.h"
#include "util/ShowTypeTestUtils.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;
using namespace rc::state;

TEST_CASE("Command") {
  SECTION("apply") {
    prop("default implementation does not modify state",
         [](const IntVec &s0) {
           auto s1 = s0;
           IntVecCmd().apply(s1);
           RC_ASSERT(s1 == s0);
         });
  }

  SECTION("run") {
    prop("default implementation does not modify SUT",
         [](const IntVec &state, const IntVec &sut) {
           const auto pre = sut;
           auto post = sut;
           IntVecCmd().run(state, post);
           RC_ASSERT(pre == post);
         });
  }

  SECTION("nextState") {
    prop("uses `apply` to calculate next state",
         [](IntVec state, int value) {
           PushBack cmd(value);
           const auto s1 = cmd.nextState(state);
           cmd.apply(state);
           RC_ASSERT(s1 == state);
         });
  }

  SECTION("show") {
    SECTION("outputs a string which contains the name of the class") {
      // TODO this is the only restriction I dare to place on this right
      // now until we can maybe get some sanitizing going
      std::ostringstream os;
      SomeCommand().show(os);
      REQUIRE(os.str().find("SomeCommand") != std::string::npos);
    }
  }
}

TEST_CASE("show(Command)") {
  prop("passing a generic command to show yields the same as Command::show",
       [] {
         PushBack cmd;
         std::ostringstream expected;
         cmd.show(expected);
         std::ostringstream actual;
         show(cmd, actual);
         RC_ASSERT(actual.str() == expected.str());
       });
}

TEST_CASE("typeToString<Command<...>>") {
  using CommandT = Command<Foo, Bar>;
  REQUIRE(typeToString<CommandT>() == "Command<FFoo, BBar>");
}
