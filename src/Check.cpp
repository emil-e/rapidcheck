#include "rapidcheck/Check.h"

#include "detail/DefaultTestListener.h"
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
                         const TestMetadata &metadata,
                         const TestParams &params) {
  return checkProperty(property, metadata, params, globalTestListener());
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
