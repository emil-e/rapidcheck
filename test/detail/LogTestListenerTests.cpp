#include <catch.hpp>

#include "detail/LogTestListener.h"

using namespace rc;
using namespace rc::detail;

TEST_CASE("LogTestListener") {
  std::ostringstream os;
  CaseDescription desc;

  SECTION("when verbose progress is off") {
    LogTestListener listener(os, false, false);

    SECTION("prints nothing when onTestCaseFinished is called") {
      desc.result.type = CaseResult::Type::Success;
      listener.onTestCaseFinished(desc);
      desc.result.type = CaseResult::Type::Discard;
      listener.onTestCaseFinished(desc);
      desc.result.type = CaseResult::Type::Failure;
      listener.onTestCaseFinished(desc);

      REQUIRE(os.str().empty());
    }
  }

  SECTION("when verbose progress is on") {
    LogTestListener listener(os, true, false);
    CaseDescription desc;

    SECTION("prints '.' on successful test case") {
      desc.result.type = CaseResult::Type::Success;
      listener.onTestCaseFinished(desc);
      REQUIRE(os.str() == ".");
    }

    SECTION("prints 'x' on discarded test case") {
      desc.result.type = CaseResult::Type::Discard;
      listener.onTestCaseFinished(desc);
      REQUIRE(os.str() == "x");
    }

    SECTION("prints something containing 'fail' on failure") {
      desc.result.type = CaseResult::Type::Failure;
      listener.onTestCaseFinished(desc);
      REQUIRE(os.str().find("fail") != std::string::npos);
    }
  }

  SECTION("when verbose shrinking is off") {
    LogTestListener listener(os, false, false);
    SECTION("prints nothing when onShrinkTried is called") {
      listener.onShrinkTried(desc, true);
      listener.onShrinkTried(desc, false);
      REQUIRE(os.str().empty());
    }
  }

  SECTION("when verbose shrinking is on") {
    LogTestListener listener(os, false, true);

    SECTION("prints '.' on non-accepted try") {
      listener.onShrinkTried(desc, false);
      REQUIRE(os.str() == ".");
    }

    SECTION("prints '!' on accepted try") {
      listener.onShrinkTried(desc, true);
      REQUIRE(os.str() == "!");
    }
  }

  SECTION("when both verbose shrinking and verbose progress is off") {
    LogTestListener listener(os, false, false);

    SECTION("never prints anything") {
      listener.onTestCaseFinished(desc);
      listener.onShrinkTried(desc, true);
      listener.onTestFinished(TestMetadata(), SuccessResult());
    }
  }
}
