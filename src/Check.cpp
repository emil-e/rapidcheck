#include "rapidcheck/Check.h"

#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/detail/TestListenerAdapter.h"

namespace rc {

void beforeMinimalTestCase() {}

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

Distribution collectTags(std::vector<Tags> allTags) {
  Distribution distribution;
  for (auto &tags : allTags) {
    if (!tags.empty()) {
      distribution[std::move(tags)]++;
    }
  }

  return distribution;
}

std::pair<CaseDescription, int>
shrinkTestCase(const Shrinkable<CaseDescription> &shrinkable,
               TestListener &listener) {
  int numShrinks = 0;
  Shrinkable<CaseDescription> best = shrinkable;

  auto shrinks = shrinkable.shrinks();
  while (auto shrink = shrinks.next()) {
    auto caseDescription = shrink->value();
    bool accept = isFailure(caseDescription);
    listener.onShrinkTried(caseDescription, accept);
    if (accept) {
      best = std::move(*shrink);
      shrinks = best.shrinks();
      numShrinks++;
    }
  }

  beforeMinimalTestCase();
  return std::make_pair(best.value(), numShrinks);
}

TestResult doCheckProperty(const Property &property,
                           const TestParams &params,
                           TestListener &listener) {
  const auto maxDiscard = params.maxDiscardRatio * params.maxSuccess;

  auto numSuccess = 0;
  auto numDiscarded = 0;
  auto recentDiscards = 0;
  auto random = Random(params.seed);

  std::vector<Tags> tags;
  tags.reserve(params.maxSuccess);

  while (numSuccess < params.maxSuccess) {
    TestCase currentCase;
    currentCase.size = sizeFor(params, numSuccess) + (recentDiscards / 10);
    currentCase.seed = avalanche(params.seed + numSuccess + recentDiscards);

    const auto shrinkable =
        property(Random(currentCase.seed), currentCase.size);
    const auto caseDescription = shrinkable.value();
    listener.onTestCaseFinished(currentCase, caseDescription);
    const auto &result = caseDescription.result;
    if (result.type == CaseResult::Type::Failure) {
      // Test case failed, shrink it
      const auto shrinkResult = shrinkTestCase(shrinkable, listener);
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
      recentDiscards++;
      if (numDiscarded > maxDiscard) {
        GaveUpResult gaveUp;
        gaveUp.numSuccess = numSuccess;
        gaveUp.description = result.description;
        return gaveUp;
      }
    } else {
      // Success!
      numSuccess++;
      recentDiscards = 0;
      tags.push_back(std::move(caseDescription.tags));
    }
  }

  SuccessResult success;
  success.numSuccess = numSuccess;
  success.distribution = collectTags(std::move(tags));
  return success;
}

} // namespace

TestResult checkProperty(const Property &property,
                         const TestParams &params,
                         TestListener &listener) {
  TestResult result = doCheckProperty(property, params, listener);
  listener.onTestFinished(result);
  return result;
}

TestResult checkProperty(const Property &property, const TestParams &params) {
  TestListenerAdapter listener;
  return checkProperty(property, params, listener);
}

} // namespace detail
} // namespace rc
