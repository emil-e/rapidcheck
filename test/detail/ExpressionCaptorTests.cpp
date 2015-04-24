#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/ExpressionCaptor.h"

#include "util/Box.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace rc {

template <>
struct Arbitrary<ExpressionCaptor> {
  static Gen<ExpressionCaptor> arbitrary() {
    return gen::map<std::string>(
        [](const std::string &str) { return ExpressionCaptor(str); });
  }
};

} // namespace rc

TEST_CASE("ExpressionCaptor") {
  SECTION("str") {
    prop("returns the current value",
         [](const std::string &value) {
           RC_ASSERT(ExpressionCaptor(value).str() == value);
         });
  }

  SECTION("operator->*") {
    prop("simply appends RHS",
         [](const ExpressionCaptor &captor, Box value) {
           RC_ASSERT((std::move(captor)->*value).str() ==
                     (captor.str() + value.str()));
         });
  }

#define TEST_BINARY_OPERATOR(op)                                               \
  SECTION("operator" #op) {                                                    \
    prop("joins LHS and RHS with operator string between",                     \
         [](const ExpressionCaptor &captor, Box value) {                       \
      RC_ASSERT((std::move(captor) op value).str() ==                          \
                (captor.str() + " " #op " " + value.str()));                   \
         });                                                                   \
  }

  TEST_BINARY_OPERATOR(*)
  TEST_BINARY_OPERATOR(/ )
  TEST_BINARY_OPERATOR(% )

  TEST_BINARY_OPERATOR(+)
  TEST_BINARY_OPERATOR(-)

  TEST_BINARY_OPERATOR(<< )
  TEST_BINARY_OPERATOR(>> )

  TEST_BINARY_OPERATOR(< )
  TEST_BINARY_OPERATOR(> )
  TEST_BINARY_OPERATOR(>= )
  TEST_BINARY_OPERATOR(<= )

  TEST_BINARY_OPERATOR(== )
  TEST_BINARY_OPERATOR(!= )

  TEST_BINARY_OPERATOR(&)

  TEST_BINARY_OPERATOR (^)

  TEST_BINARY_OPERATOR(| )

  TEST_BINARY_OPERATOR(&&)

  TEST_BINARY_OPERATOR(|| )

#undef TEST_BINARY_OPERATOR
}
