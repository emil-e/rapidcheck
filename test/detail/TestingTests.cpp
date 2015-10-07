#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/TestListenerAdapter.h"
#include "detail/Testing.h"

#include "util/Generators.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/MockTestListener.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

template <typename Testable>
SearchResult searchTestable(Testable &&testable, const TestParams &params) {
  TestListenerAdapter listener;
  return searchProperty(
      toProperty(std::forward<Testable>(testable)), params, listener);
}

template <typename Testable>
SearchResult searchTestable(Testable &&testable,
                            const TestParams &params,
                            TestListener &listener) {
  return searchProperty(
      toProperty(std::forward<Testable>(testable)), params, listener);
}

TEST_CASE("searchProperty") {
  prop("reports correct number of successes and discards",
       [](const TestParams &params, int limit) {
         int successes = 0;
         int discards = 0;
         const auto result = searchTestable([&] {
           const auto x = *gen::arbitrary<int>();
           if ((x % 3) == 0) {
             discards++;
             RC_DISCARD("");
           }
           RC_ASSERT(x < limit);
           successes++;
         }, params);

         RC_ASSERT(result.numSuccess == successes);
         RC_ASSERT(result.numDiscarded == discards);
       });

  prop("runs all test cases if no cases fail",
       [](const TestParams &params) {
         auto numCases = 0;
         const auto result = searchTestable([&] { numCases++; }, params);
         RC_ASSERT(numCases == params.maxSuccess);
         RC_ASSERT(result.type == SearchResult::Type::Success);
         RC_ASSERT(result.numSuccess == params.maxSuccess);
         RC_ASSERT(!result.failure);
       });

  prop("returns correct information about failing case",
       [](const TestParams &params, const std::string &description) {
         RC_PRE(params.maxSuccess > 0);
         auto caseIndex = 0;
         const auto targetSuccess = *gen::inRange<int>(0, params.maxSuccess);
         const auto result = searchTestable([&] {
           if (caseIndex >= targetSuccess) {
             return CaseResult(CaseResult::Type::Failure, description);
           }
           caseIndex++;
           return CaseResult(CaseResult::Type::Success);
         }, params);
         RC_ASSERT(result.type == SearchResult::Type::Failure);
         RC_ASSERT(result.failure);
         RC_ASSERT(result.failure->value().result.description == description);
       });

  prop("gives up if too many test cases are discarded",
       [](const TestParams &params, const std::string &description) {
         RC_PRE(params.maxSuccess > 0);
         const auto maxDiscards = params.maxSuccess * params.maxDiscardRatio;
         const auto targetSuccess = *gen::inRange<int>(0, params.maxSuccess);
         int numTests = 0;
         const auto result = searchTestable([&] {
           numTests++;
           if (numTests > targetSuccess) {
             return CaseResult(CaseResult::Type::Discard, description);
           }
           return CaseResult(CaseResult::Type::Success);
         }, params);

         RC_ASSERT(result.type == SearchResult::Type::GaveUp);
         RC_ASSERT(result.failure);
         RC_ASSERT(result.failure->value().result.description == description);
       });

  prop("does not give up if not enough tests are discarded",
       [](const TestParams &params) {
         const auto maxDiscards = params.maxSuccess * params.maxDiscardRatio;
         const auto targetDiscard = *gen::inRange<int>(0, maxDiscards + 1);
         int numTests = 0;
         const auto result = searchTestable([&] {
           numTests++;
           RC_PRE(numTests > targetDiscard);
         }, params);

         RC_ASSERT(result.type == SearchResult::Type::Success);
         RC_ASSERT(result.numSuccess == params.maxSuccess);
       });

  prop("if maxSuccess > 1, the max size used is maxSize",
       [](const TestParams &params) {
         RC_PRE(params.maxSuccess > 1);

         int usedMax = 0;
         const auto property = [&] { usedMax = std::max(*genSize(), usedMax); };
         searchTestable(property, params);
         RC_ASSERT(usedMax == params.maxSize);
       });

  prop("maxSuccess > maxSize, all sizes will be used",
       [] {
         TestParams params;
         params.maxSize = *gen::inRange(0, 100);
         params.maxSuccess = *gen::inRange(params.maxSuccess + 1, 200);

         std::vector<int> frequencies(params.maxSize + 1, 0);
         const auto property = [&] { frequencies[*genSize()]++; };
         searchTestable(property, params);

         RC_ASSERT(std::count(begin(frequencies), end(frequencies), 0) == 0);
       });

  prop("should increase size eventually if enough tests are discarded",
       [](TestParams params) {
         params.maxDiscardRatio = 100;
         const auto result =
             searchTestable([] { RC_PRE(*genSize() != 0); }, params);
         RC_ASSERT(result.type == SearchResult::Type::Success);
       });

  prop("does not include empty tags in tags",
       [](const TestParams &params) {
         const auto result = searchTestable([] {}, params);
         RC_ASSERT(result.tags.empty());
       });

  prop("does not include tags for discarded tests in tags",
       [](const TestParams &params) {
         const auto result = searchTestable([] {
           RC_TAG(0);
           RC_DISCARD("");
         }, params);
         RC_ASSERT(result.tags.empty());
       });

  prop("does not include tags for failed tests in tags",
       [](const TestParams &params) {
         const auto result = searchTestable([] {
           RC_TAG(0);
           RC_FAIL("");
         }, params);
         RC_ASSERT(result.tags.empty());
       });

  prop("does not include empty tags in tags",
       [](const TestParams &params) {
         const auto expected = *gen::container<std::vector<Tags>>(
                                   params.maxSuccess, gen::nonEmpty<Tags>());
         std::size_t i = 0;
         const auto result = searchTestable([&] {
           for (const auto &tag : expected[i++]) {
             ImplicitParam<param::CurrentPropertyContext>::value()->addTag(tag);
           }
         }, params);

         RC_ASSERT(result.tags == expected);
       });

  prop("does not include tags applied from generators",
       [](const TestParams &params) {
         const auto result = searchTestable([&] {
           *Gen<int>([](const Random &, int) {
             ImplicitParam<param::CurrentPropertyContext>::value()->addTag(
                 "foobar");
             return shrinkable::just(1337);
           });
         }, params);

         RC_ASSERT(result.tags.empty());
       });

  prop("calls onTestCaseFinished for each successful test",
       [](const TestParams &params, int limit) {
         std::vector<CaseDescription> descriptions;
         MockTestListener listener;
         listener.onTestCaseFinishedCallback =
             [&](const CaseDescription &desc) { descriptions.push_back(desc); };

         std::vector<CaseDescription> expected;
         const auto result = searchTestable([&] {
           CaseDescription desc;
           const auto x = *gen::arbitrary<int>();
           if ((x % 3) == 0) {
             desc.result.type = CaseResult::Type::Discard;
           } else if (x < limit) {
             desc.result.type = CaseResult::Type::Failure;
           } else {
             desc.result.type = CaseResult::Type::Success;
           }

           const auto strx = std::to_string(x);
           desc.result.description = strx;
           desc.tags.push_back(strx);
           desc.example = [=] { return Example{{"int", strx}}; };
           ImplicitParam<param::CurrentPropertyContext>::value()->addTag(strx);
           expected.push_back(desc);

           return desc.result;
         }, params, listener);

         RC_ASSERT(descriptions == expected);
       });
}

Shrinkable<CaseDescription> countdownEven(int start) {
  return shrinkable::map(countdownShrinkable(start),
                         [=](int x) {
                           CaseDescription desc;
                           desc.result.type = ((x % 2) == 0)
                               ? CaseResult::Type::Failure
                               : CaseResult::Type::Success;
                           desc.result.description = std::to_string(x);
                           return desc;
                         });
}

TEST_CASE("shrinkTestCase") {
  prop("returns the minimum shrinkable",
       [] {
         const auto target = *gen::positive<int>();
         const auto shrinkable =
             shrinkable::map(shrinkable::shrinkRecur(
                                 std::numeric_limits<int>::max(),
                                 [](int x) { return shrink::towards(x, 0); }),
                             [=](int x) {
                               CaseDescription desc;
                               desc.result.type = (x >= target)
                                   ? CaseResult::Type::Failure
                                   : CaseResult::Type::Success;
                               desc.result.description = std::to_string(x);
                               return desc;
                             });

         TestListenerAdapter listener;
         const auto result = shrinkTestCase(shrinkable, listener);
         RC_ASSERT(result.first.value().result.type ==
                   CaseResult::Type::Failure);
         RC_ASSERT(result.first.value().result.description ==
                   std::to_string(target));
       });

  prop("the path length is the number of successful shrinks",
       [] {
         const auto start = *gen::suchThat(gen::inRange<int>(0, 100),
                                           [](int x) { return (x % 2) == 0; });
         const auto shrinkable = countdownEven(start);

         TestListenerAdapter listener;
         const auto result = shrinkTestCase(shrinkable, listener);
         RC_ASSERT(result.second.size() == start / 2);
       });

  prop("walking the path gives the same result",
       [] {
         const auto start = *gen::suchThat(gen::inRange<int>(0, 100),
                                           [](int x) { return (x % 2) == 0; });
         const auto shrinkable = countdownEven(start);

         TestListenerAdapter listener;
         const auto shrinkResult = shrinkTestCase(shrinkable, listener);
         const auto walkResult =
             shrinkable::walkPath(shrinkable, shrinkResult.second);
         RC_ASSERT(walkResult);
         RC_ASSERT(shrinkResult.first.value() == walkResult->value());
       });

  prop("calls onShrinkTried for each shrink tried",
       [] {
         const auto start = *gen::suchThat(gen::inRange<int>(0, 100),
                                           [](int x) { return (x % 2) == 0; });
         const auto shrinkable = countdownEven(start);

         MockTestListener listener;
         int acceptedBalance = 0;
         listener.onShrinkTriedCallback =
             [&](const CaseDescription &desc, bool accepted) {
               const auto x = std::stoi(desc.result.description);
               RC_ASSERT(((x % 2) == 0) == accepted);
               acceptedBalance += accepted ? 1 : -1;
             };
         const auto result = shrinkTestCase(shrinkable, listener);
       });
}
