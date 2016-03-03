#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

#include <typeindex>

#include "util/GenUtils.h"
#include "util/IntVec.h"
#include "util/NonCopyableModel.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

TEST_CASE("state::isValidCommand") {
  SECTION("returns true for valid commands") {
    REQUIRE(isValidCommand(PushBack(1337), IntVec()));
  }

  SECTION("returns false for invalid commands") {
    REQUIRE(!isValidCommand(PopBack(), IntVec()));
  }
}

namespace {

struct A : public IntVecCmd {};
struct B : public IntVecCmd {};
struct C : public IntVecCmd {};

struct StateConstructible : public IntVecCmd {
  StateConstructible() = default;
  StateConstructible(const IntVec &s)
      : state(s)
      , generated(*gen::just(s)) {}

  IntVec state;
  IntVec generated;
};

struct ArgsConstructible : public IntVecCmd {
  ArgsConstructible() = default;
  ArgsConstructible(const std::string &s, int n)
      : str(s)
      , num(n) {}

  std::string str;
  int num = 0;
};

template <typename T>
struct GeneratesOnConstrution : public IntVecCmd {
  GeneratesOnConstrution(const T &v)
      : value(*gen::just(v)) {}

  T value;
};

struct AlwaysDiscard : public IntVecCmd {
  AlwaysDiscard() { RC_DISCARD("Nope"); }
};

} // namespace

// Test for deprecated function, don't warn
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif // __GNUC__

TEST_CASE("state::gen::execOneOf") {
  prop("returns one of the commands",
       [](const GenParams &params, const IntVec &s0) {
         const auto cmd =
             state::gen::execOneOf<A, B, C>(s0)(params.random, params.size)
                 .value();
         const auto &cmdRef = *cmd;
         const auto &id = typeid(cmdRef);
         RC_ASSERT((id == typeid(A)) || (id == typeid(B)) || (id == typeid(C)));
       });

  prop("all commands are eventually returned",
       [](const GenParams &params, const IntVec &s0) {
         auto r = params.random;
         const auto gen = state::gen::execOneOf<A, B, C>(s0);
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
       [](const GenParams &params, const IntVec &s0) {
         const auto cmd = state::gen::execOneOf<StateConstructible>(s0)(
                              params.random, params.size)
                              .value();
         RC_ASSERT(static_cast<const StateConstructible &>(*cmd).state == s0);
       });

  prop("allows use of operator*",
       [](const GenParams &params, const IntVec &s0) {
         using Cmd = GeneratesOnConstrution<IntVec>;
         const auto cmd =
             state::gen::execOneOf<Cmd>(s0)(params.random, params.size).value();
         RC_ASSERT(static_cast<const Cmd &>(*cmd).value == s0);
       });
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

TEST_CASE("state::gen::execOneOfWithArgs") {
  prop("returns one of the commands",
       [](const GenParams &params, const IntVec &s0) {
         const auto cmd = state::gen::execOneOfWithArgs<A, B, C>()()(
                              params.random, params.size)
                              .value();
         const auto &cmdRef = *cmd;
         const auto &id = typeid(cmdRef);
         RC_ASSERT((id == typeid(A)) || (id == typeid(B)) || (id == typeid(C)));
       });

  prop("all commands are eventually returned",
       [](const GenParams &params, const IntVec &s0) {
         auto r = params.random;
         const auto gen = state::gen::execOneOfWithArgs<A, B, C>()();
         std::set<std::type_index> all{typeid(A), typeid(B), typeid(C)};
         std::set<std::type_index> generated;
         while (generated != all) {
           const auto cmd = gen(r.split(), params.size).value();
           const auto &cmdRef = *cmd;
           generated.emplace(typeid(cmdRef));
         }
         RC_SUCCEED("All generated");
       });

  prop("uses args constructor if there is one, passing it the state",
       [](const GenParams &params, const std::string &str, int num) {
         const auto cmd = state::gen::execOneOfWithArgs<ArgsConstructible>()(
                              str, num)(params.random, params.size)
                              .value();

         const auto &dualCmd = static_cast<const ArgsConstructible &>(*cmd);
         RC_ASSERT(dualCmd.str == str);
         RC_ASSERT(dualCmd.num == num);
       });

  prop("allows use of operator*",
       [](const GenParams &params, const std::string &str) {
         using Cmd = GeneratesOnConstrution<std::string>;
         const auto cmd = state::gen::execOneOfWithArgs<Cmd>()(str)(
                              params.random, params.size)
                              .value();
         RC_ASSERT(static_cast<const Cmd &>(*cmd).value == str);
       });
}

TEST_CASE("state::check") {
  prop("if no command fails, check succeeds",
       [](const IntVec &s0, IntVec sut) {
         state::check(s0, sut, &state::gen::execOneOf<PushBack>);
       });

  prop("if some command fails, check fails",
       [](const IntVec &s0, IntVec sut) {
         try {
           state::check(s0, sut, &state::gen::execOneOf<AlwaysFail>);
           RC_FAIL("Check succeeded");
         } catch (const CaseResult &result) {
           RC_ASSERT(result.type == CaseResult::Type::Failure);
         }
       });

  prop("works with non-copyable models",
       [](const GenParams &params) {
         auto sut = initialNonCopyableModel();
         state::check(
             &initialNonCopyableModel,
             sut,
             [](const NonCopyableModel &model) {
               return state::gen::execOneOfWithArgs<NonCopyableInc,
                                                    NonCopyableDec>()(
                   model.value);
             });
       });
}

// TODO rename test file to match source files
