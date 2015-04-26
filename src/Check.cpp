#include "rapidcheck/Check.h"

#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Configuration.h"

namespace rc {
namespace detail {
namespace {

bool isFailure(const CaseDescription &desc) {
  return desc.result.type == CaseResult::Type::Failure;
}

int sizeFor(const TestParams &params, std::size_t i) {
  // We want sizes to be evenly spread, even when maxSuccess is not an even
  // multiple of the number of sizes (i.e. maxSize + 1). Another thing is that
  // we always want to ensure that the maximum size is actually used.

  const auto numSizes = params.maxSize + 1;
  const auto numRegular = (params.maxSuccess / numSizes) * numSizes;
  if (i < numRegular) {
    return i % numSizes;
  }

  const auto numRest = params.maxSuccess - numRegular;
  if (numRest == 1) {
    return 0;
  } else {
    return ((i % numSizes) * params.maxSize) / (numRest - 1);
  }
}

} // namespace

TestResult checkProperty(const Property &property, const TestParams &params) {
  int maxDiscard = params.maxDiscardRatio * params.maxSuccess;
  int numDiscarded = 0;
  int numSuccess = 0;
  int index = 0;
  std::size_t totalTests = 0;

  TestCase currentCase;
  currentCase.size = sizeFor(params, index);

  for (; numSuccess < params.maxSuccess; totalTests++) {
    currentCase.seed = avalanche(params.seed + totalTests);

    const auto shrinkable =
        property(Random(currentCase.seed), currentCase.size);
    const auto result = shrinkable.value().result;
    if (result.type == CaseResult::Type::Failure) {
      // Test case failed, shrink it
      const auto shrinkResult =
          shrinkable::findLocalMin(shrinkable, &isFailure);
      const auto &caseDesc = shrinkResult.first;
      FailureResult failure;
      failure.numSuccess = numSuccess;
      failure.failingCase = currentCase;
      failure.description = caseDesc.result.description;
      failure.numShrinks = shrinkResult.second;
      failure.counterExample = caseDesc.example;
      return failure;
    } else if (result.type == CaseResult::Type::Discard) {
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
      currentCase.size = sizeFor(params, ++index);
    }
  }

  SuccessResult success;
  success.numSuccess = numSuccess;
  return success;
}

} // namespace detail
} // namespace rc
