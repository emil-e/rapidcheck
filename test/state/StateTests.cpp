#include <catch.hpp>
#include <rapidcheck/catch.h>
#include <rapidcheck/state.h>

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

TEST_CASE("state::check") {
  prop("if no command fails, check succeeds",
       [](const IntVec &s0, IntVec sut) {
         state::check(s0, sut, state::gen::execOneOfWithArgs<PushBack>());
       });

  prop("if some command fails, check fails",
       [](const IntVec &s0, IntVec sut) {
         try {
           state::check(s0, sut, state::gen::execOneOfWithArgs<AlwaysFail>());
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
