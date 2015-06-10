#include <catch.hpp>
#include <rapidcheck-catch.h>
#include <rapidcheck/state.h>

#include <typeindex>

#include "util/StringVec.h"
#include "util/GenUtils.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::state::detail;
using namespace rc::state;
using namespace rc::test;

TEST_CASE("state::isValidCommand") {
  SECTION("returns true for valid commands") {
    REQUIRE(isValidCommand(PushBack("foo"), StringVec()));
  }

  SECTION("returns false for invalid commands") {
    REQUIRE(!isValidCommand(PopBack(), StringVec()));
  }
}

namespace {

struct A : public StringVecCmd {};
struct B : public StringVecCmd {};
struct C : public StringVecCmd {};

struct DualConstructible : public StringVecCmd {
  DualConstructible();
  DualConstructible(const StringVec &s)
      : state(s) {}
  StringVec state;
};

struct AlwaysDiscard : public StringVecCmd {
  AlwaysDiscard() { RC_DISCARD("Nope"); }
};

} // namespace

TEST_CASE("state::anyCommand") {
  prop("returns one of the commands",
       [](const GenParams &params, const StringVec &s0) {
         const auto cmd =
             anyCommand<A, B, C>(s0)(params.random, params.size).value();
         const auto &cmdRef = *cmd;
         const auto &id = typeid(cmdRef);
         RC_ASSERT((id == typeid(A)) || (id == typeid(B)) || (id == typeid(C)));
       });

  prop("all commands are eventually returned",
       [](const GenParams &params, const StringVec &s0) {
         auto r = params.random;
         const auto gen = anyCommand<A, B, C>(s0);
         std::set<std::type_index> all{typeid(A), typeid(B), typeid(C)};
         std::set<std::type_index> generated;
         while (generated != all) {
           const auto cmd = gen(r.split(), params.size).value();
           const auto &cmdRef = *cmd;
           generated.emplace(typeid(cmdRef));
         }
         RC_SUCCEED("All generated");
       });

  prop("uses state constructor if there is one, passing it the state",
       [](const GenParams &params, const StringVec &s0) {
         const auto cmd =
             anyCommand<DualConstructible>(s0)(params.random, params.size)
                 .value();
         RC_ASSERT(static_cast<const DualConstructible &>(*cmd).state == s0);
       });
}

TEST_CASE("state::check") {
  prop("if no command fails, check succeeds",
       [](const StringVec &s0, StringVec sut) {
         state::check(s0, sut, &anyCommand<PushBack>);
       });

  prop("if some command fails, check fails",
       [](const StringVec &s0, StringVec sut) {
         try {
           state::check(s0, sut, &anyCommand<AlwaysFail>);
           RC_FAIL("Check succeeded");
         } catch (const CaseResult &result) {
           RC_ASSERT(result.type == CaseResult::Type::Failure);
         }
       });
}
