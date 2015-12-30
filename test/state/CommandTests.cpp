#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include "util/StringVec.h"
#include "util/ShowTypeTestUtils.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;
using namespace rc::state;

TEST_CASE("Command") {
  SECTION("apply") {
    prop("default implementation does not modify state",
         [](const StringVec &s0) {
           auto s1 = s0;
           StringVecCmd().apply(s1);
           RC_ASSERT(s1 == s0);
         });
  }

  SECTION("run") {
    prop("default implementation fails",
         [](const StringVec &state, const StringVec &sut) {
           auto post = sut;
           RC_ASSERT_THROWS_AS(StringVecCmd().run(state, post), CaseResult);
         });

    prop("default implementation of alternative overload fails",
         [](const StringVec &state, const StringVec &sut) {
           auto post = sut;
           RC_ASSERT_THROWS_AS(StringVecCmd().run(post), CaseResult);
         });
  }

  SECTION("nextState") {
    prop("uses `apply` to calculate next state",
         [](StringVec state, const std::string &value) {
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
