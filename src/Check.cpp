#include "rapidcheck/Check.h"

#include "rapidcheck/detail/LogTestListener.h"
#include "detail/Testing.h"

namespace rc {
namespace detail {

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params,
                         TestListener &listener) {
  return testProperty(property, metadata, params, listener);
}

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata) {
  const auto &config = configuration();
  LogTestListener listener(
      std::cerr, config.verboseProgress, config.verboseShrinking);
  return testProperty(property, metadata, config.testParams, listener);
}

TestResult checkProperty(const Property &property) {
  return checkProperty(property, TestMetadata());
}

} // namespace detail
} // namespace rc
