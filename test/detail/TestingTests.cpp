#include <catch.hpp>
#include <rapidcheck/catch.h>

#include <algorithm>

#include "rapidcheck/detail/TestListenerAdapter.h"
#include "detail/Testing.h"

#include "util/Generators.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/MockTestListener.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace {

TestListenerAdapter dummyListener;

} // namespace

template <typename Testable>
SearchResult searchTestable(Testable &&testable,
                            const TestParams &params,
                            TestListener &listener = dummyListener) {
  return searchProperty(
      toProperty(std::forward<Testable>(testable)), params, listener);
}

template <typename Testable>
TestResult testTestable(Testable &&testable,
                        const TestParams &params,
                        TestListener &listener = dummyListener) {
  return testProperty(toProperty(std::forward<Testable>(testable)),
                      TestMetadata(),
                      params,
                      dummyListener);
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
         int size = 0;
         int caseIndex = 0;
         const auto targetSuccess = *gen::inRange<int>(0, params.maxSuccess);
         const auto result = searchTestable([&] {
           size = *genSize();
           if (caseIndex >= targetSuccess) {
             return CaseResult(CaseResult::Type::Failure, description);
           }
           caseIndex++;
           return CaseResult(CaseResult::Type::Success);
         }, params);

         RC_ASSERT(result.type == SearchResult::Type::Failure);
         RC_ASSERT(result.failure);
         RC_ASSERT(result.failure->size == size);
         RC_ASSERT(result.failure->shrinkable.value().result.description ==
                   description);
       });

  prop("gives up if too many test cases are discarded",
       [](const TestParams &params, const std::string &description) {
         RC_PRE(params.maxSuccess > 0);
         const auto targetSuccess = *gen::inRange<int>(0, params.maxSuccess);
         int size = 0;
         int numTests = 0;
         const auto result = searchTestable([&] {
           numTests++;
           size = *genSize();
           if (numTests > targetSuccess) {
             return CaseResult(CaseResult::Type::Discard, description);
           }
           return CaseResult(CaseResult::Type::Success);
         }, params);

         RC_ASSERT(result.type == SearchResult::Type::GaveUp);
         RC_ASSERT(result.failure);
         RC_ASSERT(result.failure->size == size);
         RC_ASSERT(result.failure->shrinkable.value().result.description ==
                   description);
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

  prop("if maxSuccess > maxSize, all sizes will be used",
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

  prop("the failure information reproduces identical shrinkables",
       [](TestParams params) {
         const auto max = *gen::inRange<int>(0, 2000);
         const auto property = toProperty([=](int a, int b) {
           if ((a > max) || (b > max)) {
             throw std::to_string(a) + " " + std::to_string(b);
           }
         });

         params.maxSuccess = 2000;
         params.maxSize = kNominalSize;

         const auto result = searchProperty(property, params, dummyListener);
         RC_ASSERT(result.failure);

         const auto shrinkable =
             property(result.failure->random, result.failure->size);
         RC_ASSERT(result.failure->shrinkable.value() == shrinkable.value());
       });
}

namespace {
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

         const auto result = shrinkTestCase(shrinkable, dummyListener);
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

         const auto result = shrinkTestCase(shrinkable, dummyListener);
         RC_ASSERT(result.second.size() == std::size_t(start / 2));
       });

  prop("walking the path gives the same result",
       [] {
         const auto start = *gen::suchThat(gen::inRange<int>(0, 100),
                                           [](int x) { return (x % 2) == 0; });
         const auto shrinkable = countdownEven(start);

         const auto shrinkResult = shrinkTestCase(shrinkable, dummyListener);
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

TEST_CASE("testProperty") {
  prop("returns the correct shrink path on a failing case",
       [](TestParams params) {
         RC_PRE(params.maxSuccess > 0);
         params.disableShrinking = false;
         const auto evenInteger =
             gen::scale(0.25,
                        gen::suchThat(gen::positive<int>(),
                                      [](int x) { return (x % 2) == 0; }));
         const auto values = *gen::pair(evenInteger, evenInteger);
         const auto results = testTestable([&] {
           const auto v1 = *genFixedCountdown(values.first);
           const auto v2 = *genFixedCountdown(values.second);
           return ((v1 % 2) != 0) || ((v2 % 2) != 0);
         }, params, dummyListener);

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         const auto numShrinks = (values.first / 2) + (values.second / 2);
         // Every shrink should be the second shrink, thus fill with 1
         const auto expected = std::vector<std::size_t>(numShrinks, 1);
         RC_ASSERT(failure.reproduce.shrinkPath == expected);
       });

  prop("returns a correct counter-example",
       [](const TestParams &params, std::vector<int> values) {
         RC_PRE(params.maxSuccess > 0);
         const auto results =
             testTestable([&](FixedCountdown<0>, FixedCountdown<0>) {
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
         const auto results = testTestable([&] {
           *gen::just<std::string>("foo");
           auto innerResults = testTestable([&] {
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
         const auto results = testTestable(
             [&] { RC_FAIL(description); }, params, dummyListener);

         FailureResult failure;
         RC_ASSERT(results.match(failure));
         RC_ASSERT(failure.description.find(description) != std::string::npos);
       });

  prop("on giving up, description contains message",
       [](const TestParams &params, const std::string &description) {
         RC_PRE(params.maxSuccess > 0);
         const auto results = testTestable(
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

         const auto results1 = testTestable(property, params, dummyListener);
         auto values1 = std::move(values);

         values = std::vector<std::vector<int>>();
         const auto results2 = testTestable(property, params, dummyListener);
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
         params.maxSuccess = static_cast<int>(allTags.size());

         auto i = 0;
         const auto property = [&] {
           const auto &tags = allTags[i++];
           for (const auto &tag : tags) {
             ImplicitParam<param::CurrentPropertyContext>::value()->addTag(tag);
           }
         };
         const auto result = testTestable(property, params, dummyListener);

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
         const auto result = testTestable([] {}, params, dummyListener);
         SuccessResult success;
         RC_ASSERT(result.match(success));
         RC_ASSERT(success.distribution.empty());
       });

  prop("does not shrink result if disableShrinking is set",
       [](TestParams params) {
         RC_PRE(params.maxSuccess > 0);

         params.disableShrinking = true;
         const auto result = testTestable([] {
           *Gen<int>([](const Random &, int) {
             return shrinkable::just(1337, seq::just(shrinkable::just(0)));
           });
           RC_FAIL("oh noes");
         }, params, dummyListener);

         FailureResult failure;
         RC_ASSERT(result.match(failure));
         RC_ASSERT(failure.counterExample.front().second == "1337");
       });
}

TEST_CASE("reproduceProperty") {
  prop("reproduces result from testProperty",
       [](const TestMetadata &metadata, TestParams params) {
         const auto max = *gen::inRange<int>(0, 2000);
         const auto property = toProperty([=](int a, int b) {
           if ((a > max) || (b > max)) {
             throw std::to_string(a) + " " + std::to_string(b);
           }
         });

         params.maxSuccess = 2000;
         params.maxSize = kNominalSize;

         const auto result =
             testProperty(property, metadata, params, dummyListener);
         FailureResult failure;
         RC_ASSERT(result.match(failure));

         const auto reproduced = reproduceProperty(property, failure.reproduce);
         FailureResult reproducedFailure;
         RC_ASSERT(reproduced.match(reproducedFailure));

         RC_ASSERT(failure.description == reproducedFailure.description);
         RC_ASSERT(failure.reproduce == reproducedFailure.reproduce);
         RC_ASSERT(failure.counterExample == reproducedFailure.counterExample);
         RC_ASSERT(reproducedFailure.numSuccess == 0);
       });

  SECTION("returns error if reproduced result is not a failure") {
    const auto property = toProperty([] {});
    Reproduce repro;
    repro.size = 0;
    const auto result = reproduceProperty(property, repro);
    Error error;
    REQUIRE(result.match(error));
  }

  SECTION("returns error if shrink path is not valid") {
    const auto property = toProperty([] { return false; });
    Reproduce repro;
    repro.size = 0;
    repro.shrinkPath.push_back(100);
    const auto result = reproduceProperty(property, repro);
    Error error;
    REQUIRE(result.match(error));
  }
}
