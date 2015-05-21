#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Util.h"
#include "util/Generators.h"
#include "util/TemplateProps.h"
#include "util/GenUtils.h"

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Check.h"
#include "rapidcheck/gen/Create.h"
#include "rapidcheck/gen/Container.h"
#include "rapidcheck/gen/Numeric.h"
#include "rapidcheck/Classify.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

// TODO good candidate for profiling

TEST_CASE("checkTestable") {
  prop("runs all test cases if no cases fail",
       [](const TestParams &params) {
         auto numCases = 0;
         const auto result = checkTestable([&] { numCases++; }, params);
         RC_ASSERT(numCases == params.maxSuccess);

         SuccessResult success;
         RC_ASSERT(result.match(success));
         RC_ASSERT(success.numSuccess == params.maxSuccess);
       });

  prop("returns correct information about failing case",
       [](const TestParams &params) {
         RC_PRE(params.maxSuccess > 0);
         auto caseIndex = 0;
         auto lastSize = -1;
         const auto targetSuccess = *gen::inRange<int>(0, params.maxSuccess);
         const auto result = checkTestable([&] {
           lastSize = (*genPassedParams()).size;
           RC_ASSERT(caseIndex < targetSuccess);
           caseIndex++;
         }, params);
         FailureResult failure;
         RC_ASSERT(result.match(failure));
         RC_ASSERT(failure.numSuccess == targetSuccess);
         RC_ASSERT(failure.failingCase.size == lastSize);
       });

  prop("returns the correct number of shrinks on a failing case",
       [] {
         const auto evenInteger =
             gen::scale(0.25,
                        gen::suchThat(gen::positive<int>(),
                                      [](int x) { return (x % 2) == 0; }));
         const auto values = *gen::pair(evenInteger, evenInteger);
         const auto results = checkTestable([&] {
           const auto v1 = *genFixedCountdown(values.first);
           const auto v2 = *genFixedCountdown(values.second);
           return ((v1 % 2) != 0) || ((v2 % 2) != 0);
         });

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         RC_ASSERT(failure.numShrinks ==
                   ((values.first / 2) + (values.second / 2)));
       });

  prop("returns a correct counter-example",
       [](std::vector<int> values) {
         const auto results =
             checkTestable([&](FixedCountdown<0>, FixedCountdown<0>) {
               for (auto value : values) {
                 *gen::just(value);
               }
               return false;
             });

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
       [] {
         const auto results = checkTestable([] {
           *gen::just<std::string>("foo");
           auto innerResults = checkTestable([&] {
             *gen::just<std::string>("bar");
             *gen::just<std::string>("baz");
           });

           return false;
         });

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         Example expected{
             {typeToString<std::tuple<>>(), toString(std::tuple<>{})},
             {typeToString<std::string>(), toString(std::string("foo"))}};
         RC_ASSERT(failure.counterExample == expected);
       });

  prop("on failure, description contains message",
       [](const std::string &description) {
         const auto results = checkTestable([&] { RC_FAIL(description); });

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         RC_ASSERT(failure.description.find(description) != std::string::npos);
       });

  prop("gives up if too many test cases are discarded",
       [](const TestParams &params) {
         RC_PRE(params.maxSuccess > 0);
         const auto maxDiscards = params.maxSuccess * params.maxDiscardRatio;
         const auto targetSuccess = *gen::inRange<int>(0, params.maxSuccess);
         int numTests = 0;
         const auto results = checkTestable([&] {
           numTests++;
           RC_PRE(numTests <= targetSuccess);
         }, params);
         RC_ASSERT(numTests >= (targetSuccess + maxDiscards));

         GaveUpResult gaveUp;
         RC_ASSERT(results.match(gaveUp));
         RC_ASSERT(gaveUp.numSuccess == targetSuccess);
       });

  prop("does not give up if not enough tests are discarded",
       [](const TestParams &params) {
         const auto maxDiscards = params.maxSuccess * params.maxDiscardRatio;
         const auto targetDiscard = *gen::inRange<int>(0, maxDiscards + 1);
         int numTests = 0;
         const auto results = checkTestable([&] {
           numTests++;
           RC_PRE(numTests > targetDiscard);
         }, params);

         SuccessResult success;
         RC_ASSERT(results.match(success));
         RC_ASSERT(success.numSuccess == params.maxSuccess);
       });

  prop("on giving up, description contains message",
       [](const std::string &description) {
         const auto results = checkTestable([&] { RC_DISCARD(description); });

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

         const auto results1 = checkTestable(property, params);
         auto values1 = std::move(values);

         values = std::vector<std::vector<int>>();
         const auto results2 = checkTestable(property, params);
         auto values2 = std::move(values);

         RC_ASSERT(results1 == results2);
         RC_ASSERT(values1 == values2);
       });

  prop("if maxSuccess > 1, the max size used is maxSize",
       [](const TestParams &params) {
         RC_SUCCEED_IF(params.maxSuccess <= 1);

         int usedMax = 0;
         const auto property = [&] { usedMax = std::max(*genSize(), usedMax); };
         checkTestable(property, params);
         RC_ASSERT(usedMax == params.maxSize);
       });

  prop("maxSuccess > maxSize, all sizes will be used",
       [] {
         TestParams params;
         params.maxSize = *gen::inRange(0, 100);
         params.maxSuccess = *gen::inRange(params.maxSuccess + 1, 200);

         std::vector<int> frequencies(params.maxSize + 1, 0);
         const auto property = [&] { frequencies[*genSize()]++; };
         checkTestable(property, params);

         RC_ASSERT(std::count(begin(frequencies), end(frequencies), 0) == 0);
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
         const auto result = checkTestable(property, params);

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
         const auto result = checkTestable([] {}, params);
         SuccessResult success;
         RC_ASSERT(result.match(success));
         RC_ASSERT(success.distribution.empty());
       });
}
