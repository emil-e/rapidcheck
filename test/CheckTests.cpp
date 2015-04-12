#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Util.h"
#include "util/Generators.h"
#include "util/TemplateProps.h"
#include "util/GenUtils.h"

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Check.h"
#include "rapidcheck/newgen/Create.h"
#include "rapidcheck/newgen/Container.h"
#include "rapidcheck/newgen/Numeric.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace rc {

template<>
class NewArbitrary<TestParams>
{
public:
    static Gen<TestParams> arbitrary()
    {
        return newgen::exec([]{
            TestParams params;
            params.maxSuccess = *newgen::inRange(0, 100);
            params.maxSize = *newgen::inRange(0, 101);
            params.maxDiscardRatio = *newgen::inRange(0, 100);
            return params;
        });
    }
};

} // namespace rc

TEST_CASE("TestParams") {
    SECTION("operator==/operator!=") {
        newpropConformsToEquals<TestParams>();
        NEWPROP_REPLACE_MEMBER_INEQUAL(TestParams, seed);
        NEWPROP_REPLACE_MEMBER_INEQUAL(TestParams, maxSuccess);
        NEWPROP_REPLACE_MEMBER_INEQUAL(TestParams, maxSize);
        NEWPROP_REPLACE_MEMBER_INEQUAL(TestParams, maxDiscardRatio);
    }
}

TEST_CASE("defaultTestParams") {
    newprop(
        "takes default params from ImplicitParam<CurrentConfiguration>",
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

// TODO good candidate for profiling

TEST_CASE("newCheckTestable") {
    newprop(
        "runs all test cases if no cases fail",
        [] (const TestParams &params) {
            int numCases = 0;
            auto result = newCheckTestable([&] {
                numCases++;
            }, params);
            RC_ASSERT(numCases == params.maxSuccess);

            SuccessResult success;
            RC_ASSERT(result.match(success));
            RC_ASSERT(success.numSuccess == params.maxSuccess);
        });

    newprop(
        "returns correct information about failing case",
        [] (const TestParams &params) {
            RC_PRE(params.maxSuccess > 0);
            int caseIndex = 0;
            int lastSize = -1;
            int targetSuccess = *newgen::inRange<int>(0, params.maxSuccess);
            auto result = newCheckTestable([&] {
                lastSize = (*genPassedParams()).size;
                RC_ASSERT(caseIndex < targetSuccess);
                caseIndex++;
            }, params);
            FailureResult failure;
            RC_ASSERT(result.match(failure));
            RC_ASSERT(failure.numSuccess == targetSuccess);
            RC_ASSERT(failure.failingCase.size == lastSize);
        });

    newprop(
        "returns the correct number of shrinks on a failing case",
        [] {
            auto evenInteger = newgen::scale(
                0.25,
                newgen::suchThat(
                    newgen::positive<int>(),
                    [](int x) { return (x % 2) == 0; }));
            auto values = *newgen::pair(evenInteger, evenInteger);
            auto results = newCheckTestable([&] {
                auto v1 = *genFixedCountdown(values.first);
                auto v2 = *genFixedCountdown(values.second);
                return ((v1 % 2) != 0) || ((v2 % 2) != 0);
            });

            FailureResult failure;
            RC_ASSERT(results.match(failure));
            RC_ASSERT(failure.numShrinks == ((values.first / 2) +
                                             (values.second / 2)));
        });

    newprop(
        "returns a correct counter-example",
        [] (std::vector<int> values) {
            auto results = newCheckTestable(
                [&](FixedCountdown<0>, FixedCountdown<0>) {
                    for (auto value : values)
                        *newgen::just(value);
                    return false;
                });

            Example expected;
            expected.reserve(values.size() + 1);
            std::tuple<FixedCountdown<0>, FixedCountdown<0>> expectedArgs(
                FixedCountdown<0>{}, FixedCountdown<0>{});
            expected.push_back(
                std::make_pair(typeToString<decltype(expectedArgs)>(),
                               toString(expectedArgs)));
            std::transform(
                begin(values), end(values),
                std::back_inserter(expected),
                [](int x) {
                    return std::make_pair(typeToString<int>(),
                                          toString(x));
                });

            FailureResult failure;
            RC_ASSERT(results.match(failure));
            RC_ASSERT(failure.counterExample == expected);
        });

    newprop(
        "counter-example is not affected by nested tests",
        [] {
            auto results = newCheckTestable([] {
                *newgen::just<std::string>("foo");
                auto innerResults = newCheckTestable([&] {
                    *newgen::just<std::string>("bar");
                    *newgen::just<std::string>("baz");
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

    newprop(
        "on failure, description contains message",
        [] (const std::string &description) {
            auto results = newCheckTestable([&] {
                RC_FAIL(description);
            });

            FailureResult failure;
            RC_ASSERT(results.match(failure));
            RC_ASSERT(failure.description.find(description) !=
                      std::string::npos);
        });

    newprop(
        "gives up if too many test cases are discarded",
        [] (const TestParams &params) {
            RC_PRE(params.maxSuccess > 0);
            const int maxDiscards = params.maxSuccess * params.maxDiscardRatio;
            const int targetSuccess = *newgen::inRange<int>(0, params.maxSuccess);
            int numTests = 0;
            auto results = newCheckTestable([&] {
                numTests++;
                RC_PRE(numTests <= targetSuccess);
            }, params);
            RC_ASSERT(numTests >= (targetSuccess + maxDiscards));

            GaveUpResult gaveUp;
            RC_ASSERT(results.match(gaveUp));
            RC_ASSERT(gaveUp.numSuccess == targetSuccess);
        });

    newprop(
        "does not give up if not enough tests are discarded",
        [] (const TestParams &params) {
            const int maxDiscards = params.maxSuccess * params.maxDiscardRatio;
            const int targetDiscard = *newgen::inRange<int>(0, maxDiscards + 1);
            int numTests = 0;
            auto results = newCheckTestable([&] {
                numTests++;
                RC_PRE(numTests > targetDiscard);
            }, params);

            SuccessResult success;
            RC_ASSERT(results.match(success));
            RC_ASSERT(success.numSuccess == params.maxSuccess);
        });

    newprop(
        "on giving up, description contains message",
        [] (const std::string &description) {
            auto results = newCheckTestable([&] {
                RC_DISCARD(description);
            });

            GaveUpResult gaveUp;
            RC_ASSERT(results.match(gaveUp));
            RC_ASSERT(gaveUp.description.find(description) !=
                      std::string::npos);
        });

    newprop(
        "running the same test with the same TestParams yields identical runs",
        [] (const TestParams &params) {
            std::vector<std::vector<int>> values;
            auto newproperty
                = [&] {
                auto x = *newgen::arbitrary<std::vector<int>>();
                values.push_back(x);
                auto result = std::find(begin(x), end(x), 50);
                return result == end(x);
            };

            auto results1 = newCheckTestable(newproperty,
                                             params);
            auto values1 = std::move(values);

            values = std::vector<std::vector<int>>();
            auto results2 = newCheckTestable(newproperty,
                                             params);
            auto values2 = std::move(values);

            RC_ASSERT(results1 == results2);
            RC_ASSERT(values1 == values2);
        });
}
