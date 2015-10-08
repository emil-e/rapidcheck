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
       [](const TestParams &params, int limit) {
         Maybe<TestResult> callbackResult;
         MockTestListener listener;
         listener.onTestFinishedCallback = [&](const TestResult &result) {
           RC_ASSERT(!callbackResult);
           callbackResult.init(result);
         };

         const auto result = checkTestable([&] {
           const auto x = *gen::arbitrary<int>();
           if ((x % 3) == 0) {
             RC_DISCARD("");
           }
           RC_ASSERT(x < limit);
         }, params, listener);

         RC_ASSERT(callbackResult);
         RC_ASSERT(*callbackResult == result);
       });
}
