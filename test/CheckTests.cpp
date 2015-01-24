#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Util.h"

// TODO fix when we have configurability
#define DEFAULT_NUM_TESTS 100
#define DEFAULT_MAX_SIZE 100
#define DEFAULT_MAX_DISCARDS 10

using namespace rc;
using namespace rc::detail;

class CountDownGenerator : public gen::Generator<int>
{
public:
    CountDownGenerator(int value)
        : m_value(value) {}

    int generate() const override
    { return m_value; }

    shrink::IteratorUP<int> shrink(int value) const override
    {
        if (value == 0)
            return shrink::nothing<int>();

        return shrink::unfold(
            value,
            [](int x) { return x >= 0; },
            [](int x) { return std::make_pair(x - 1, x - 1); });
    }

private:
    const int m_value;
};

TEST_CASE("checkTestable") {
    SECTION("runs all test cases if no cases fail") {
        int numCases = 0;
        int lastSize = -1;
        auto result = checkTestable([&] {
            numCases++;
        });
        REQUIRE(numCases == DEFAULT_NUM_TESTS);

        SuccessResult success;
        REQUIRE(result.match(success));
        REQUIRE(success.numTests == DEFAULT_NUM_TESTS);
    }

    prop("returns correct information about failing case",
         [] {
             int numCases = 0;
             int lastSize = -1;
             // TODO test seed?
             int failingIndex = pick(gen::ranged<int>(1, DEFAULT_NUM_TESTS));
             auto result = checkTestable([&] {
                 numCases++;
                 lastSize = gen::currentSize();
                 return numCases < failingIndex;
             });
             FailureResult failure;
             RC_ASSERT(result.match(failure));
             RC_ASSERT(failure.failingCase.index == failingIndex);
             RC_ASSERT(failure.failingCase.size == lastSize);
         });

    prop("returns the correct number of shrinks on a failing case",
         [] {
             auto evenInteger = gen::scale(
                 0.25,
                 gen::suchThat(
                     gen::positive<int>(),
                     [](int x) { return (x % 2) == 0; }));
             auto values = pick(gen::pairOf(evenInteger, evenInteger));
             auto results = checkTestable([&] {
                 auto v1 = pick(CountDownGenerator(values.first));
                 auto v2 = pick(CountDownGenerator(values.second));
                 return ((v1 % 2) != 0) || ((v2 % 2) != 0);
             });

             FailureResult failure;
             RC_ASSERT(results.match(failure));
             RC_ASSERT(failure.numShrinks == ((values.first / 2) +
                                              (values.second / 2)));
         });

    prop("returns a correct counter-example",
         [] (std::vector<int> values) {
             auto results = checkTestable([&] {
                 for (auto value : values)
                     pick(gen::constant(value));
                 return false;
             });

             FailureResult failure;
             RC_ASSERT(results.match(failure));
             RC_ASSERT(failure.counterExample.size() == values.size());
             for (int i = 0; i < values.size(); i++) {
                 RC_ASSERT(failure.counterExample[i] ==
                           ValueDescription(values[i]));
             }
         });

    prop("counter-example is not affected by nested tests",
         [] {
             auto results = checkTestable([] {
                 pick(gen::constant<std::string>("foo"));
                 auto innerResults = checkTestable([&] {
                     pick(gen::constant<std::string>("bar"));
                     pick(gen::constant<std::string>("baz"));
                 });

                 return false;
             });

             FailureResult failure;
             RC_ASSERT(results.match(failure));
             RC_ASSERT(failure.counterExample.size() == 1);
             RC_ASSERT(failure.counterExample[0] ==
                       ValueDescription(std::string("foo")));
         });

    prop("on failure, description contains message",
         [] (const std::string &description) {
             auto results = checkTestable([&] {
                 RC_FAIL(description);
             });

             FailureResult failure;
             RC_ASSERT(results.match(failure));
             RC_ASSERT(failure.description.find(description) !=
                       std::string::npos);
         });

    prop("gives up if too many test cases are discarded",
         [] {
             const int maxDiscards = DEFAULT_NUM_TESTS * DEFAULT_MAX_DISCARDS;
             const int targetSuccess = pick(gen::ranged<int>(1, DEFAULT_NUM_TESTS));
             int numTests = 0;
             auto results = checkTestable([&] {
                 numTests++;
                 RC_PRE(numTests <= targetSuccess);
             });
             RC_ASSERT(numTests >= (targetSuccess + maxDiscards));

             GaveUpResult gaveUp;
             RC_ASSERT(results.match(gaveUp));
             RC_ASSERT(gaveUp.numTests == targetSuccess);
         });

    prop("does not give up if not enough tests are discarded",
         [] {
             const int maxDiscards = DEFAULT_NUM_TESTS * DEFAULT_MAX_DISCARDS;
             const int targetDiscard = pick(gen::ranged<int>(0, maxDiscards + 1));
             int numTests = 0;
             auto results = checkTestable([&] {
                 numTests++;
                 RC_PRE(numTests > targetDiscard);
             });

             SuccessResult success;
             RC_ASSERT(results.match(success));
             RC_ASSERT(success.numTests == DEFAULT_NUM_TESTS);
         });

    prop("on giving up, description contains message",
         [] (const std::string &description) {
             auto results = checkTestable([&] {
                 RC_DISCARD(description);
             });

             GaveUpResult gaveUp;
             RC_ASSERT(results.match(gaveUp));
             RC_ASSERT(gaveUp.description.find(description) !=
                       std::string::npos);
         });
}
