#include "rapidcheck/Check.h"

#include "rapidcheck/gen/Generator.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/GenerationParams.h"
#include "rapidcheck/detail/Rose.h"

namespace rc {
namespace detail {

std::ostream &operator<<(std::ostream &os, const TestParams &params)
{
    os << "maxSuccess=" << params.maxSuccess;
    os << ", maxSize=" << params.maxSize;
    os << ", maxDiscardRatio=" << params.maxDiscardRatio;
    return os;
}

template<typename Callable>
auto withTestCase(const TestCase &testCase, Callable callable)
    -> decltype(callable())
{
    RandomEngine engine(testCase.seed);
    ImplicitParam<param::CurrentNode> currentNode(nullptr);
    ImplicitParam<param::RandomEngine> randomEngine(&engine);
    ImplicitParam<param::Size> size(testCase.size);

    return callable();
}

TestResult shrinkFailingCase(const gen::Generator<CaseResult> &property,
                             const TestCase &testCase)
{
    Rose<CaseResult> rose(&property, testCase);
    FailureResult result;
    result.failingCase = testCase;
    result.numShrinks = 0;

    bool didShrink = true;
    while (true) {
        CaseResult shrinkResult(rose.nextShrink(didShrink));
        if (didShrink) {
            if (shrinkResult.type() == CaseResult::Type::Failure) {
                rose.acceptShrink();
                result.numShrinks++;
            }
        } else {
            result.description = shrinkResult.description();
            result.counterExample = rose.example();
            return result;
        }
    }
}

TestResult checkProperty(const gen::Generator<CaseResult> &property,
                         const TestParams &params)
{
    using namespace detail;
    // NOTE: Fresh scope
    ImplicitScope scope;

    TestCase currentCase;

    int maxDiscard = params.maxDiscardRatio * params.maxSuccess;
    int numDiscarded = 0;
    currentCase.size = 0;
    currentCase.index = 1;
    while (currentCase.index <= params.maxSuccess) {
        CaseResult result = withTestCase(
            currentCase,
            [&]{ return property.generate(); });

        if (result.type() == CaseResult::Type::Failure) {
            return shrinkFailingCase(property, currentCase);
        } else if(result.type() == CaseResult::Type::Discard) {
            numDiscarded++;
            if (numDiscarded > maxDiscard) {
                return GaveUpResult {
                    .numTests = (currentCase.index - 1),
                    .description = result.description() };
            }
        } else {
            currentCase.index++;
            currentCase.size = (currentCase.size + 1) % (params.maxSize + 1);
            //TODO better size calculation
        }
    }

    return SuccessResult{ .numTests = (currentCase.index - 1) };
}

} // namespace detail
} // namespace rc
