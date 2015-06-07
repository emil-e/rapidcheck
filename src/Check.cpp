#include "rapidcheck/Check.h"

#include "rapidcheck/detail/LogTestlistener.h"
#include "detail/Testing.h"

namespace rc {

/// Dummy function to set breakpoint on before running minimal test case.
void beforeMinimalTestCase() {}

namespace detail {
namespace {

TestResult doCheckProperty(const Property &property,
                           const TestParams &params,
                           TestListener &listener) {
  const auto searchResult = searchProperty(property, params, listener);
  if (searchResult.type == SearchResult::Type::Success) {
    SuccessResult success;
    success.numSuccess = searchResult.numSuccess;
    for (const auto &tags : searchResult.tags) {
      success.distribution[tags]++;
    }
    return success;
  } else if (searchResult.type == SearchResult::Type::GaveUp) {
    GaveUpResult gaveUp;
    gaveUp.numSuccess = searchResult.numSuccess;
    gaveUp.description =
        std::move(searchResult.failure->value().result.description);
    return gaveUp;
  } else {
    // Shrink it unless shrinking is disabled
    const auto shrinkResult = params.disableShrinking
        ? std::make_pair(*searchResult.failure, 0)
        : shrinkTestCase(*searchResult.failure, listener);
    beforeMinimalTestCase();
    const auto caseDescription = shrinkResult.first.value();

    FailureResult failure;
    failure.numSuccess = searchResult.numSuccess;
    failure.description = std::move(caseDescription.result.description);
    failure.numShrinks = shrinkResult.second;
    failure.counterExample = std::move(caseDescription.example);
    return failure;
  }
}

} // namespace

TestResult checkProperty(const Property &property,
                         const TestParams &params,
                         TestListener &listener) {
  TestResult result = doCheckProperty(property, params, listener);
  listener.onTestFinished(result);
  return result;
}

TestResult checkProperty(const Property &property) {
  const auto config = ImplicitParam<param::CurrentConfiguration>::value();
  LogTestListener listener(
      std::cerr, config.verboseProgress, config.verboseShrinking);
  return checkProperty(property, config.testParams, listener);
}

} // namespace detail
} // namespace rc
