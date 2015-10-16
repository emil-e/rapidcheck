#include "rapidcheck/Check.h"

#include "rapidcheck/detail/LogTestListener.h"
#include "detail/Testing.h"

namespace rc {
namespace detail {

namespace {

TestListener &defaultTestListener() {
  const auto &config = configuration();
  static LogTestListener listener(
      std::cerr, config.verboseProgress, config.verboseShrinking);
  return listener;
}

} // namespace

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params,
                         TestListener &listener) {
  return testProperty(property, metadata, params, listener);
}

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata,
                         const TestParams &params) {
  return checkProperty(property, metadata, params, defaultTestListener());
}

TestResult checkProperty(const Property &property,
                         const TestMetadata &metadata) {
  return checkProperty(property, metadata, configuration().testParams);
}

TestResult checkProperty(const Property &property) {
  return checkProperty(property, TestMetadata());
}

} // namespace detail
} // namespace rc
