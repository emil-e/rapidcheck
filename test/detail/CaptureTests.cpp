#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Capture.h"

#include "util/Box.h"

using namespace rc;
using namespace rc::test;

namespace {

template <typename Expression, typename T>
void verifyExpression(const Expression &expression,
                      const T &expectedValue,
                      const std::string &expectedExpansion) {
  SECTION(expectedExpansion) {
    REQUIRE(expression.value() == expectedValue);

    std::ostringstream ss;
    expression.show(ss);
    REQUIRE(ss.str() == expectedExpansion);
  }
}

#define VERIFY_EXPRESSION(expression, expansion)                               \
  verifyExpression(RC_INTERNAL_CAPTURE(expression), (expression), expansion)

} // namespace

namespace rc {

// We're intentionally leaving out parenthesis for testing capture of multiple
// operators here so suppress GCC warnings for that:
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#endif // __GNUC__

TEST_CASE("RC_INTERNAL_CAPTURE") {
  SECTION("*") { VERIFY_EXPRESSION(2 * 3 * 4, "2 * 3 * 4"); }

  SECTION("/") { VERIFY_EXPRESSION(2 / 3, "2 / 3"); }

  SECTION("%") { VERIFY_EXPRESSION(2 % 3, "2 % 3"); }

  SECTION("+") {
    VERIFY_EXPRESSION(2 + 3 + 4, "2 + 3 + 4");
    VERIFY_EXPRESSION(std::string("foo") + std::string("\n"),
                      "\"foo\" + \"\\n\"");
  }

  SECTION("-") { VERIFY_EXPRESSION(2 - 3 - 4, "2 - 3 - 4"); }

  SECTION("<<") { VERIFY_EXPRESSION(1 << 4, "1 << 4"); }

  SECTION("<<") { VERIFY_EXPRESSION(53 >> 4, "53 >> 4"); }

  SECTION("<") {
    VERIFY_EXPRESSION(1 < 2, "1 < 2");
    VERIFY_EXPRESSION(std::string("foo") < std::string("\n"),
                      "\"foo\" < \"\\n\"");
  }

  SECTION(">") {
    VERIFY_EXPRESSION(1 > 2, "1 > 2");
    VERIFY_EXPRESSION(std::string("foo") > std::string("\n"),
                      "\"foo\" > \"\\n\"");
  }

  SECTION(">=") {
    VERIFY_EXPRESSION(1 >= 2, "1 >= 2");
    VERIFY_EXPRESSION(std::string("foo") >= std::string("\n"),
                      "\"foo\" >= \"\\n\"");
  }

  SECTION("<=") {
    VERIFY_EXPRESSION(1 <= 2, "1 <= 2");
    VERIFY_EXPRESSION(std::string("foo") <= std::string("\n"),
                      "\"foo\" <= \"\\n\"");
  }

  SECTION("==") {
    VERIFY_EXPRESSION(1 == 2, "1 == 2");
    VERIFY_EXPRESSION(std::string("foo") == std::string("\n"),
                      "\"foo\" == \"\\n\"");
  }

  SECTION("!=") {
    VERIFY_EXPRESSION(1 != 2, "1 != 2");
    VERIFY_EXPRESSION(std::string("foo") != std::string("\n"),
                      "\"foo\" != \"\\n\"");
  }

  SECTION("&") { VERIFY_EXPRESSION(3 & 1, "3 & 1"); }

  SECTION("^") { VERIFY_EXPRESSION(1 ^ 2, "1 ^ 2"); }

  SECTION("|") { VERIFY_EXPRESSION(1 | 2, "1 | 2"); }

  SECTION("&&") { VERIFY_EXPRESSION(true && false, "true && false"); }

  SECTION("||") { VERIFY_EXPRESSION(true || false, "true || false"); }

  SECTION("mixed") {
    VERIFY_EXPRESSION(2 + 3 == 5, "2 + 3 == 5");
    VERIFY_EXPRESSION(2 % 3 == 5 == true, "2 % 3 == 5 == true");
  }
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

} // namespace rc
