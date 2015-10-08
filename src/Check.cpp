#include "rapidcheck/Check.h"

#include "rapidcheck/detail/LogTestListener.h"
#include "detail/Testing.h"

namespace rc {
namespace detail {

TestResult checkProperty(const Property &property,
                         const TestParams &params,
                         TestListener &listener) {
  return testProperty(property, params, listener);
}

TestResult checkProperty(const Property &property) {
  const auto config = ImplicitParam<param::CurrentConfiguration>::value();
  LogTestListener listener(
      std::cerr, config.verboseProgress, config.verboseShrinking);
  return checkProperty(property, config.testParams, listener);
}

} // namespace detail
} // namespace rc
