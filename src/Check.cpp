#include "rapidcheck/Check.h"

#include "rapidcheck/detail/TestListenerAdapter.h"
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
  if (searchResult.type == SearchResult::Type::Ok) {
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
    // Shrink it
    const auto shrinkResult = shrinkTestCase(*searchResult.failure, listener);
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

TestResult checkProperty(const Property &property, const TestParams &params) {
  TestListenerAdapter listener;
  return checkProperty(property, params, listener);
}

} // namespace detail
} // namespace rc
