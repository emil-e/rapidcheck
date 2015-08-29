#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "util/Generators.h"

using namespace rc;
using namespace rc::detail;

namespace {

bool stringContains(const std::string &str, const std::string &substr) {
  return str.find(substr) != std::string::npos;
}

bool descriptionContains(const CaseResult &result, const std::string &substr) {
  return stringContains(result.description, substr);
}

} // namespace

TEST_CASE("makeDescriptionMessage") {
  SECTION("message contains description") {
    REQUIRE(stringContains(makeDescriptionMessage("", 0, "foo bar baz"),
                           "foo bar baz"));
  }

  SECTION("message contains file and line") {
    REQUIRE(stringContains(makeDescriptionMessage("foo.cpp", 1337, ""),
                           "foo.cpp:1337"));
  }
}

TEST_CASE("makeExpressionMessage") {
  SECTION("message contains assertion") {
    REQUIRE(stringContains(makeExpressionMessage("", 0, "ASSERT_IT(foo)", ""),
                           "ASSERT_IT(foo)"));
  }

  SECTION("message contains expansion") {
    REQUIRE(
        stringContains(makeExpressionMessage("", 0, "", "a == b"), "a == b"));
  }

  SECTION("message contains file and line") {
    REQUIRE(stringContains(makeExpressionMessage("foo.cpp", 1337, "", ""),
                           "foo.cpp:1337"));
  }
}

TEST_CASE("doAssert") {
  SECTION("does nothing if expression equals expected result") {
    doAssert(
        RC_INTERNAL_CAPTURE(true), true, CaseResult::Type::Failure, "", 0, "");
  }

  prop(
      "throws CaseResult of given type if expression does not equal expected "
      "result",
      [](CaseResult::Type type) {
        try {
          doAssert(RC_INTERNAL_CAPTURE(true), false, type, "", 0, "");
        } catch (const CaseResult &result) {
          RC_SUCCEED_IF(result.type == type);
        }
        RC_FAIL("Did not throw");
      });
}

TEST_CASE("assertions") {
  // We want to test that we never evaluate expression components more than once
  // so we pass x++ as a component to test for that.
  int x = 0;

  SECTION("RC_ASSERT") {
    SECTION("does not throw if expression is true") { RC_ASSERT(100 == 100); }

    SECTION("when false, throws Failure with relevant info") {
      try {
        RC_ASSERT(x++ == 100);
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Failure);
        REQUIRE(descriptionContains(result, "0 == 100"));
        REQUIRE(descriptionContains(result, "RC_ASSERT(x++ == 100)"));
      }
    }
  }

  SECTION("RC_ASSERT_FALSE") {
    SECTION("does not throw if expression is false") {
      RC_ASSERT_FALSE(100 == 101);
    }

    SECTION("when true, throws Failure with relevant info") {
      try {
        RC_ASSERT_FALSE(x++ == 0);
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Failure);
        REQUIRE(descriptionContains(result, "0 == 0"));
        REQUIRE(descriptionContains(result, "RC_ASSERT_FALSE(x++ == 0)"));
      }
    }
  }

  SECTION("RC_FAIL") {
    SECTION("throws Failure with message") {
      try {
        RC_FAIL("foo bar baz");
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Failure);
        REQUIRE(descriptionContains(result, "foo bar baz"));
      }
    }
  }

  SECTION("RC_SUCCEED_IF") {
    SECTION("does not throw if expression is false") {
      RC_SUCCEED_IF(100 == 101);
    }

    SECTION("when true, throws Success with relevant info") {
      try {
        RC_SUCCEED_IF(x++ == 0);
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Success);
        REQUIRE(descriptionContains(result, "0 == 0"));
        REQUIRE(descriptionContains(result, "RC_SUCCEED_IF(x++ == 0)"));
      }
    }
  }

  SECTION("RC_SUCCEED") {
    SECTION("throws Success with message") {
      try {
        RC_SUCCEED("foo bar baz");
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Success);
        REQUIRE(descriptionContains(result, "foo bar baz"));
      }
    }
  }

  SECTION("RC_PRE") {
    SECTION("does not throw if expression is true") { RC_PRE(100 == 100); }

    SECTION("when false, throws Discard with relevant info") {
      try {
        RC_PRE(x++ == 100);
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Discard);
        REQUIRE(descriptionContains(result, "0 == 100"));
        REQUIRE(descriptionContains(result, "RC_PRE(x++ == 100)"));
      }
    }
  }

  SECTION("RC_DISCARD") {
    SECTION("throws Discard with message") {
      try {
        RC_DISCARD("foo bar baz");
        FAIL("Never threw");
      } catch (const CaseResult &result) {
        REQUIRE(result.type == CaseResult::Type::Discard);
        REQUIRE(descriptionContains(result, "foo bar baz"));
      }
    }
  }
}
