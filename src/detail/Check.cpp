#include "rapidcheck/Check.h"

#include "rapidcheck/gen/Generator.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/GenerationParams.h"
#include "rapidcheck/detail/Rose.h"
#include "rapidcheck/detail/Configuration.h"

namespace rc {
namespace detail {
namespace {

template<typename Callable>
auto withTestCase(const TestCase &testCase, Callable callable)
    -> decltype(callable())
{
    ImplicitParam<param::CurrentNode> currentNode(nullptr);
    ImplicitParam<param::Random> random(Random(testCase.seed));
    ImplicitParam<param::Size> size(testCase.size);

    return callable();
}

struct ShrinkResult
{
    int numShrinks;
    std::string description;
    std::vector<std::pair<std::string, std::string>> counterExample;
};

ShrinkResult shrinkFailingCase(const gen::Generator<CaseResult> &property,
                               const TestCase &testCase)
{
    Rose<CaseResult> rose(&property, testCase);
    ShrinkResult result;
    result.numShrinks = 0;

    bool didShrink = true;
    while (true) {
        CaseResult shrinkResult(rose.nextShrink(didShrink));
        if (didShrink) {
            if (shrinkResult.type == CaseResult::Type::Failure) {
                rose.acceptShrink();
                result.numShrinks++;
            }
        } else {
            result.description = shrinkResult.description;
            result.counterExample = rose.example();
            return result;
        }
    }
}

} // namespace

TestResult checkProperty(const gen::Generator<CaseResult> &property,
                         const TestParams &params)
{
    using namespace detail;
    // NOTE: Fresh scope
    ImplicitScope scope;

    TestCase currentCase;
    currentCase.size = 0;

    int maxDiscard = params.maxDiscardRatio * params.maxSuccess;
    int numDiscarded = 0;
    int numSuccess = 0;
    int caseIndex = 0;
    while (numSuccess < params.maxSuccess) {
        // The seed is a hash of all that identifies the case together with the
        // global seed
        currentCase.seed = avalanche(params.seed +
                                     caseIndex +
                                     currentCase.size);

        CaseResult result = withTestCase(
            currentCase,
            [&]{ return property.generate(); });

        if (result.type == CaseResult::Type::Failure) {
            // Test case failed, shrink it
            auto shrinkResult = shrinkFailingCase(property, currentCase);
            return FailureResult {
                .numSuccess = numSuccess,
                .failingCase = currentCase,
                .description = std::move(shrinkResult.description),
                .numShrinks = shrinkResult.numShrinks,
                .counterExample = std::move(shrinkResult.counterExample)
            };
        } else if(result.type == CaseResult::Type::Discard) {
            // Test case discarded
            numDiscarded++;
            if (numDiscarded > maxDiscard) {
                return GaveUpResult {
                    .numSuccess = numSuccess,
                    .description = result.description };
            }
        } else {
            // Success!
            numSuccess++;
            currentCase.size = (currentCase.size + 1) % (params.maxSize + 1);
            //TODO better size calculation
        }
        caseIndex++;
    }

    return SuccessResult{ .numSuccess = numSuccess };
}

TestResult checkProperty(const gen::Generator<CaseResult> &property)
{
    return checkProperty(property, defaultTestParams());
}

} // namespace detail
} // namespace rc
