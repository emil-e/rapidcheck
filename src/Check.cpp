#include "rapidcheck/Check.h"

#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/GenerationParams.h"
#include "rapidcheck/detail/Rose.h"

namespace rc {
namespace detail {

template<typename Callable>
auto withTestCase(const TestCase &testCase, Callable callable)
    -> decltype(callable())
{
    ImplicitParam<param::RandomEngine> randomEngine;
    RandomEngine engine;
    engine.seed(testCase.seed);
    randomEngine.let(&engine);
    ImplicitParam<param::Size> size;
    size.let(testCase.size);

    return callable();
}

TestResult shrinkFailingCase(const gen::Generator<CaseResult> &property,
                             const TestCase &testCase)
{
    Rose<CaseResult> rose(property, testCase);
    FailureResult result;
    result.failingCase = testCase;
    result.numShrinks = 0;

    bool didShrink = true;
    while (true) {
        CaseResult shrinkResult(rose.nextShrink(property, didShrink));
        if (didShrink) {
            if (shrinkResult.type() == CaseResult::Type::Failure) {
                rose.acceptShrink();
                result.numShrinks++;
            }
        } else {
            result.description = shrinkResult.description();
            result.counterExample = rose.example(property);
            return result;
        }
    }
}

//! Describes the parameters for a test.
struct TestParams
{
    //! The maximum number of successes before deciding a property passes.
    int maxSuccess = 100;
    //! The maximum size to generate.
    int maxSize = 100;
    //! The maximum allowed number of discarded tests per successful test.
    int maxDiscardRatio = 10;
};

TestResult checkProperty(const gen::Generator<CaseResult> &property)
{
    using namespace detail;
    TestParams params;
    TestCase currentCase;
    RandomEngine seedEngine;

    int maxDiscard = params.maxDiscardRatio * params.maxSuccess;
    int numDiscarded = 0;
    currentCase.size = 0;
    currentCase.index = 1;
    while (currentCase.index <= params.maxSuccess) {
        currentCase.seed = seedEngine.nextAtom();

        CaseResult result = withTestCase(
            currentCase,
            [&]{ return property.generate(); });

        if (result.type() == CaseResult::Type::Failure) {
            return shrinkFailingCase(property, currentCase);
        } else if(result.type() == CaseResult::Type::Discard) {
            numDiscarded++;
            if (numDiscarded > maxDiscard) {
                return GaveUpResult {
                    .numTests = currentCase.index,
                    .description = result.description() };
            }
            continue;
        }

        currentCase.index++;
        currentCase.size = (currentCase.size + 1) % (params.maxSize + 1);
        //TODO better size calculation
    }

    return SuccessResult{ .numTests = currentCase.index };
}

void throwResultIf(CaseResult::Type type,
                   bool condition,
                   std::string description,
                   std::string file,
                   int line)
{
    if (condition) {
        throw CaseResult(
            type, file + ":" + std::to_string(line) + ": " + description);
    }
}


} // namespace detail
} // namespace rc
