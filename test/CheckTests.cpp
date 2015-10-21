#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/TestListenerAdapter.h"

#include "util/MockTestListener.h"
#include "util/GenUtils.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace {

TestListenerAdapter dummyListener;

} // namespace

TEST_CASE("checkTestable") {
  prop("calls onTestFinished with the test results once",
       [](const TestMetadata &metadata, const TestParams &params, int limit) {
         Maybe<std::tuple<TestMetadata, TestResult>> callbackParams;
         MockTestListener listener;
         listener.onTestFinishedCallback =
             [&](const TestMetadata &metadata, const TestResult &result) {
               RC_ASSERT(!callbackParams);
               callbackParams.init(metadata, result);
             };

         const auto result = checkTestable([&] {
           const auto x = *gen::arbitrary<int>();
           if ((x % 3) == 0) {
             RC_DISCARD("");
           }
           RC_ASSERT(x < limit);
         }, metadata, params, listener);

         RC_ASSERT(callbackParams);
         RC_ASSERT(std::get<0>(*callbackParams) == metadata);
         RC_ASSERT(std::get<1>(*callbackParams) == result);
       });

  SECTION("reproducing (non-properties)") {
    TestMetadata metadata;
    TestParams params;
    std::unordered_map<std::string, Reproduce> reproMap;

    SECTION("runs property if Reproduce map is empty") {
      metadata.id = "foobar";
      const auto result =
          checkTestable([] {}, metadata, params, dummyListener, reproMap);

      SuccessResult success;
      REQUIRE(result.match(success));
      // Since the property always succeeds, if the property is actually run
      // then
      // it should have been run the maximum number of times
      REQUIRE(success.numSuccess == params.maxSuccess);
    }

    SECTION(
        "always succeeds with no cases run if ID is empty and Reproduce is "
        "non-empty") {
      metadata.id = "";
      reproMap[""] = Reproduce();
      const auto result =
          checkTestable([] {}, metadata, params, dummyListener, reproMap);

      SuccessResult success;
      REQUIRE(result.match(success));
      REQUIRE(success.numSuccess == 0);
    }

    SECTION(
        "always succeeds with no cases run if ID is not found in Reproduce "
        "map") {
      metadata.id = "foobar";
      reproMap["barfoo"] = Reproduce();
      const auto result =
          checkTestable([] {}, metadata, params, dummyListener, reproMap);

      SuccessResult success;
      REQUIRE(result.match(success));
      REQUIRE(success.numSuccess == 0);
    }
  }

  prop("reproduces result from failure",
       [](const TestMetadata &metadata, TestParams params) {
         RC_PRE(!metadata.id.empty());
         const auto max = *gen::inRange<int>(0, 2000);
         const auto testable = [=](int a, int b) {
           if ((a > max) || (b > max)) {
             throw std::to_string(a) + " " + std::to_string(b);
           }
         };

         params.maxSuccess = 2000;
         params.maxSize = kNominalSize;
         const auto result =
             checkTestable(testable,
                           metadata,
                           params,
                           dummyListener,
                           std::unordered_map<std::string, Reproduce>());

         FailureResult failure;
         RC_ASSERT(result.match(failure));

         std::unordered_map<std::string, Reproduce> reproMap{
             {metadata.id, failure.reproduce}};
         const auto reproduced =
             checkTestable(testable, metadata, params, dummyListener, reproMap);
         FailureResult reproducedFailure;
         RC_ASSERT(reproduced.match(reproducedFailure));

         RC_ASSERT(failure.description == reproducedFailure.description);
         RC_ASSERT(failure.reproduce == reproducedFailure.reproduce);
         RC_ASSERT(failure.counterExample == reproducedFailure.counterExample);
         RC_ASSERT(reproducedFailure.numSuccess == 0);
       });
}
