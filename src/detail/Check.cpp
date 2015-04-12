#include "rapidcheck/Check.h"

#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Configuration.h"

namespace rc {
namespace detail {
namespace {

bool isFailure(const CaseDescription &desc)
{ return desc.result.type == CaseResult::Type::Failure; }

} // namespace

TestResult checkProperty(const NewProperty &property, const TestParams &params)
{
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

        const auto shrinkable = property(Random(currentCase.seed),
                                         currentCase.size);
        const auto result = shrinkable.value().result;
        if (result.type == CaseResult::Type::Failure) {
            // Test case failed, shrink it
            const auto shrinkResult = shrinkable::findLocalMin(shrinkable,
                                                               &isFailure);
            const auto &caseDesc = shrinkResult.first;
            FailureResult failure;
            failure.numSuccess = numSuccess;
            failure.failingCase = currentCase;
            failure.description = caseDesc.result.description;
            failure.numShrinks = shrinkResult.second;
            failure.counterExample = caseDesc.example;
            return failure;
        } else if(result.type == CaseResult::Type::Discard) {
            // Test case discarded
            numDiscarded++;
            if (numDiscarded > maxDiscard) {
                GaveUpResult gaveUp;
                gaveUp.numSuccess = numSuccess;
                gaveUp.description = result.description;
                return gaveUp;
            }
        } else {
            // Success!
            numSuccess++;
            currentCase.size = (currentCase.size + 1) % (params.maxSize + 1);
            //TODO better size calculation
        }
        caseIndex++;
    }

    SuccessResult success;
    success.numSuccess = numSuccess;
    return success;
}

} // namespace detail
} // namespace rc
