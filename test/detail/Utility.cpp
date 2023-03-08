#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Utility.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

namespace {

TEST_CASE("makeUnsigned") {
  SECTION("in range") {
    prop("pass through",
         []() {
           int i = 1;
           size_t expected = 1;
           RC_ASSERT(makeUnsigned(i) == expected);
         });
  }
  SECTION("out of range") {
    prop("throws SignException",
         []() {
           int i = -1;
           RC_ASSERT_THROWS_AS(makeUnsigned(i), SignException);
         });
  }
}

TEST_CASE("makeSigned") {
  SECTION("in range") {
    prop("pass through",
         []() {
           size_t i = 1;
           int expected = 1;
           RC_ASSERT(makeSigned(i) == expected);
         });
  }
  SECTION("out of range") {
    prop("throws SignException",
         []() {
           size_t i = std::numeric_limits<size_t>::max();
           RC_ASSERT_THROWS_AS(makeUnsigned(i), SignException);
         });
  }
}

}
