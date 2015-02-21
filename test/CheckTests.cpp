#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Util.h"
#include "util/Generators.h"
#include "util/TemplateProps.h"

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Check.h"

using namespace rc;
using namespace rc::detail;

namespace rc {

template<>
class Arbitrary<TestParams> : public gen::Generator<TestParams>
{
public:
    TestParams generate() const override
    {
        TestParams params;
        params.maxSuccess = *gen::ranged(0, 100);
        params.maxSize = *gen::ranged(0, 101);
        params.maxDiscardRatio = *gen::ranged(0, 100);
        return params;
    }
};

} // namespace rc

namespace {

// Generates a constant value but tries to shrink it by a sequence of
// decrementing towards zero. Used for testing the correct number of shrinks are
// performed/reported.
class CountDownGenerator : public gen::Generator<int>
{
public:
    CountDownGenerator(int value)
        : m_value(value) {}

    int generate() const override
    { return m_value; }

    Seq<int> shrink(int value) const override
    {
        return seq::takeWhile(
            [](int x) { return x >= 0; },
            seq::iterate(value - 1, [](int x) { return --x; }));
    }

private:
    const int m_value;
};

} // namespace

TEST_CASE("TestParams") {
    SECTION("operator==/operator!=") {
        propConformsToEquals<TestParams>();
        PROP_REPLACE_MEMBER_INEQUAL(TestParams, seed);
        PROP_REPLACE_MEMBER_INEQUAL(TestParams, maxSuccess);
        PROP_REPLACE_MEMBER_INEQUAL(TestParams, maxSize);
        PROP_REPLACE_MEMBER_INEQUAL(TestParams, maxDiscardRatio);
    }
}

TEST_CASE("checkTestable") {
    prop("runs all test cases if no cases fail",
         [] (const TestParams &params) {
             int numCases = 0;
             int lastSize = -1;
             auto result = checkTestable([&] {
                 numCases++;
             }, params);
             RC_ASSERT(numCases == params.maxSuccess);

             SuccessResult success;
             RC_ASSERT(result.match(success));
             RC_ASSERT(success.numSuccess == params.maxSuccess);
         });

    prop("returns correct information about failing case",
         [] (const TestParams &params) {
             RC_PRE(params.maxSuccess > 0);
             int caseIndex = 0;
             int lastSize = -1;
             int targetSuccess = *gen::ranged<int>(0, params.maxSuccess);
             auto result = checkTestable([&] {
                 lastSize = gen::currentSize();
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
             auto evenInteger = gen::scale(
                 0.25,
                 gen::suchThat(
                     gen::positive<int>(),
                     [](int x) { return (x % 2) == 0; }));
             auto values = *gen::pairOf(evenInteger, evenInteger);
             auto results = checkTestable([&] {
                 auto v1 = *CountDownGenerator(values.first);
                 auto v2 = *CountDownGenerator(values.second);
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
                     *gen::constant(value);
                 return false;
             });

             FailureResult failure;
             RC_ASSERT(results.match(failure));
             RC_ASSERT(failure.counterExample.size() == values.size());
             for (int i = 0; i < values.size(); i++) {
                 RC_ASSERT(failure.counterExample[i] ==
                           std::make_pair(typeToString<int>(),
                                          toString(values[i])));
             }
         });

    prop("counter-example is not affected by nested tests",
         [] {
             auto results = checkTestable([] {
                 *gen::constant<std::string>("foo");
                 auto innerResults = checkTestable([&] {
                     *gen::constant<std::string>("bar");
                     *gen::constant<std::string>("baz");
                 });

                 return false;
             });

             FailureResult failure;
             RC_ASSERT(results.match(failure));
             RC_ASSERT(failure.counterExample.size() == 1);
             RC_ASSERT(failure.counterExample[0] ==
                       std::make_pair(typeToString<std::string>(),
                                      toString(std::string("foo"))));
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
         [] (const TestParams &params) {
             RC_PRE(params.maxSuccess > 0);
             const int maxDiscards = params.maxSuccess * params.maxDiscardRatio;
             const int targetSuccess = *gen::ranged<int>(0, params.maxSuccess);
             int numTests = 0;
             auto results = checkTestable([&] {
                 numTests++;
                 RC_PRE(numTests <= targetSuccess);
             }, params);
             RC_ASSERT(numTests >= (targetSuccess + maxDiscards));

             GaveUpResult gaveUp;
             RC_ASSERT(results.match(gaveUp));
             RC_ASSERT(gaveUp.numSuccess == targetSuccess);
         });

    prop("does not give up if not enough tests are discarded",
         [] (const TestParams &params) {
             const int maxDiscards = params.maxSuccess * params.maxDiscardRatio;
             const int targetDiscard = *gen::ranged<int>(0, maxDiscards + 1);
             int numTests = 0;
             auto results = checkTestable([&] {
                 numTests++;
                 RC_PRE(numTests > targetDiscard);
             }, params);

             SuccessResult success;
             RC_ASSERT(results.match(success));
             RC_ASSERT(success.numSuccess == params.maxSuccess);
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

    prop("running the same test with the same TestParams yields identical runs",
         [] (const TestParams &params) {
             std::vector<std::vector<int>> values;
             auto property = [&] {
                 auto x = *gen::arbitrary<std::vector<int>>();
                 values.push_back(x);
                 auto result = std::find(begin(x), end(x), 50);
                 return result == end(x);
             };

             auto results1 = checkTestable(property, params);
             auto values1 = std::move(values);

             values = std::vector<std::vector<int>>();
             auto results2 = checkTestable(property, params);
             auto values2 = std::move(values);

             RC_ASSERT(results1 == results2);
             RC_ASSERT(values1 == values2);
         });
}

TEST_CASE("defaultTestParams") {
    prop("takes default params from ImplicitParam<CurrentConfiguration>",
         [] (const Configuration &config) {
             TestParams expected;
             expected.seed = config.seed;
             expected.maxSuccess = config.maxSuccess;
             expected.maxSize = config.maxSize;
             expected.maxDiscardRatio = config.maxDiscardRatio;

             ImplicitParam<param::CurrentConfiguration> currentConfig(config);
             RC_ASSERT(defaultTestParams() == expected);
         });
}
