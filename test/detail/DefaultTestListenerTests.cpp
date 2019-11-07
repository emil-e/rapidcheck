#include <catch2/catch.hpp>

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
    auto listener = makeDefaultTestListener(config, os);
    listener->onTestCaseFinished(CaseDescription());
    listener.reset();
    REQUIRE(os.str().empty());
  }

  SECTION("should print . on test case success if verboseProgress == true") {
    Configuration config;
    config.verboseProgress = true;
    auto listener = makeDefaultTestListener(config, os);
    CaseDescription desc;
    desc.result = CaseResult(CaseResult::Type::Success);
    listener->onTestCaseFinished(desc);
    listener.reset();
    REQUIRE(os.str() == ".");
  }

  SECTION(
      "should not print anything on shrink tried if verboseShrinking == false") {
    Configuration config;
    config.verboseShrinking = false;
    auto listener = makeDefaultTestListener(config, os);
    listener->onShrinkTried(CaseDescription(), false);
    listener.reset();
    REQUIRE(os.str().empty());
  }

  SECTION("should print . on unaccepted shrink if verboseShrinking == true") {
    Configuration config;
    config.verboseShrinking = true;
    auto listener = makeDefaultTestListener(config, os);
    CaseDescription desc;
    desc.result = CaseResult(CaseResult::Type::Success);
    listener->onShrinkTried(desc, false);
    listener.reset();
    REQUIRE(os.str() == ".");
  }

  SECTION("should not have reproduce string if no failure") {
    auto listener = makeDefaultTestListener(Configuration(), os);
    TestMetadata metadata;
    metadata.id = "foobar";
    SuccessResult success;
    listener->onTestFinished(metadata, success);
    listener.reset();
    REQUIRE(os.str().find("reproduce=") == std::string::npos);
  }

  SECTION("should have reproduce string on failure") {
    auto listener = makeDefaultTestListener(Configuration(), os);
    TestMetadata metadata;
    metadata.id = "foobar";
    FailureResult failure;
    listener->onTestFinished(metadata, failure);
    listener.reset();
    REQUIRE(os.str().find("reproduce=") != std::string::npos);
  }
}
