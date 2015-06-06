#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/TestListenerAdapter.h"

#include "util/MockTestListener.h"
#include "util/GenUtils.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

// TODO good candidate for profiling

namespace {

TestListenerAdapter dummyListener;

} // namespace

TEST_CASE("checkTestable") {
  prop("returns the correct number of shrinks on a failing case",
       [](const TestParams &params) {
         RC_PRE(params.maxSuccess > 0);
         const auto evenInteger =
             gen::scale(0.25,
                        gen::suchThat(gen::positive<int>(),
                                      [](int x) { return (x % 2) == 0; }));
         const auto values = *gen::pair(evenInteger, evenInteger);
         const auto results = checkTestable([&] {
           const auto v1 = *genFixedCountdown(values.first);
           const auto v2 = *genFixedCountdown(values.second);
           return ((v1 % 2) != 0) || ((v2 % 2) != 0);
         }, params, dummyListener);

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         RC_ASSERT(failure.numShrinks ==
                   ((values.first / 2) + (values.second / 2)));
       });

  prop("returns a correct counter-example",
       [](const TestParams &params, std::vector<int> values) {
         RC_PRE(params.maxSuccess > 0);
         const auto results =
             checkTestable([&](FixedCountdown<0>, FixedCountdown<0>) {
               for (auto value : values) {
                 *gen::just(value);
               }
               return false;
             }, params, dummyListener);

         Example expected;
         expected.reserve(values.size() + 1);
         std::tuple<FixedCountdown<0>, FixedCountdown<0>> expectedArgs(
             FixedCountdown<0>{}, FixedCountdown<0>{});
         expected.push_back(std::make_pair(
             typeToString<decltype(expectedArgs)>(), toString(expectedArgs)));
         std::transform(begin(values),
                        end(values),
                        std::back_inserter(expected),
                        [](int x) {
                          return std::make_pair(typeToString<int>(),
                                                toString(x));
                        });

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         RC_ASSERT(failure.counterExample == expected);
       });

  prop("counter-example is not affected by nested tests",
       [](const TestParams &params1, const TestParams &params2) {
         RC_PRE(params1.maxSuccess > 0);
         const auto results = checkTestable([&] {
           *gen::just<std::string>("foo");
           auto innerResults = checkTestable([&] {
             *gen::just<std::string>("bar");
             *gen::just<std::string>("baz");
           }, params2, dummyListener);

           return false;
         }, params1, dummyListener);

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         Example expected{
             {typeToString<std::string>(), toString(std::string("foo"))}};
         RC_ASSERT(failure.counterExample == expected);
       });

  prop("on failure, description contains message",
       [](const TestParams &params, const std::string &description) {
         RC_PRE(params.maxSuccess > 0);
         const auto results = checkTestable(
             [&] { RC_FAIL(description); }, params, dummyListener);

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         RC_ASSERT(failure.description.find(description) != std::string::npos);
       });

  prop("on giving up, description contains message",
       [](const TestParams &params, const std::string &description) {
         RC_PRE(params.maxSuccess > 0);
         const auto results = checkTestable(
             [&] { RC_DISCARD(description); }, params, dummyListener);

         GaveUpResult gaveUp;
         RC_ASSERT(results.match(gaveUp));
         RC_ASSERT(gaveUp.description.find(description) != std::string::npos);
       });

  prop("running the same test with the same TestParams yields identical runs",
       [](const TestParams &params) {
         std::vector<std::vector<int>> values;
         const auto property = [&] {
           const auto x = *gen::arbitrary<std::vector<int>>();
           values.push_back(x);
           auto result = std::find(begin(x), end(x), 50);
           return result == end(x);
         };

         const auto results1 = checkTestable(property, params, dummyListener);
         auto values1 = std::move(values);

         values = std::vector<std::vector<int>>();
         const auto results2 = checkTestable(property, params, dummyListener);
         auto values2 = std::move(values);

         RC_ASSERT(results1 == results2);
         RC_ASSERT(values1 == values2);
       });

  prop("correctly reports test case distribution",
       [] {
         auto allTags =
             *gen::container<std::vector<std::vector<std::string>>>(
                 gen::scale(0.1, gen::arbitrary<std::vector<std::string>>()));
         TestParams params;
         params.maxSize = *gen::inRange(0, 200);
         params.maxSuccess = allTags.size();

         auto i = 0;
         const auto property = [&] {
           const auto &tags = allTags[i++];
           for (const auto &tag : tags) {
             ImplicitParam<param::CurrentPropertyContext>::value()->addTag(tag);
           }
         };
         const auto result = checkTestable(property, params, dummyListener);

         Distribution expected;
         for (auto &tags : allTags) {
           if (!tags.empty()) {
             expected[tags]++;
           }
         }

         SuccessResult success;
         RC_ASSERT(result.match(success));
         RC_ASSERT(success.distribution == expected);
       });

  prop("does not include untagged cases in distribution",
       [](const TestParams &params) {
         const auto result = checkTestable([] {}, params, dummyListener);
         SuccessResult success;
         RC_ASSERT(result.match(success));
         RC_ASSERT(success.distribution.empty());
       });

  prop("calls onTestFinished with the test results once",
       [](const TestParams &params, int limit) {
         Maybe<TestResult> callbackResult;
         MockTestListener listener;
         listener.onTestFinishedCallback = [&](const TestResult &result) {
           RC_ASSERT(!callbackResult);
           callbackResult = result;
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
