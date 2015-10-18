#include <catch.hpp>

#include "detail/DefaultTestListener.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("makeDefaultTestListener") {
  std::ostringstream os;

  SECTION(
      "should not print anything on test case finished if verboseProgress == "
      "false") {
    Configuration config;
    config.verboseProgress = false;
    const auto listener = makeDefaultTestListener(config, os);
    listener->onTestCaseFinished(CaseDescription());
    REQUIRE(os.str().empty());
  }

  SECTION("should print . on test case success if verboseProgress == true") {
    Configuration config;
    config.verboseProgress = true;
    const auto listener = makeDefaultTestListener(config, os);
    CaseDescription desc;
    desc.result = CaseResult(CaseResult::Type::Success);
    listener->onTestCaseFinished(desc);
    REQUIRE(os.str() == ".");
  }

  SECTION(
      "should not print anything on shrink tried if verboseShrinking == false") {
    Configuration config;
    config.verboseShrinking = false;
    const auto listener = makeDefaultTestListener(config, os);
    listener->onShrinkTried(CaseDescription(), false);
    REQUIRE(os.str().empty());
  }

  SECTION("should print . on unaccepted shrink if verboseShrinking == true") {
    Configuration config;
    config.verboseShrinking = true;
    const auto listener = makeDefaultTestListener(config, os);
    CaseDescription desc;
    desc.result = CaseResult(CaseResult::Type::Success);
    listener->onShrinkTried(desc, false);
    REQUIRE(os.str() == ".");
  }
}
