#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/TestListenerAdapter.h"

#include "util/MockTestListener.h"
#include "util/GenUtils.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

// TODO good candidate for profiling

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
}
