#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/shrinkable/Create.h"

#include "util/Logger.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("shrinkable::lambda") {
  SECTION("calls the value callable when value is called each time") {
    int calledTimes = 0;
    const auto shrinkable = shrinkable::lambda(
        [&] { return calledTimes++; }, [] { return Seq<Shrinkable<int>>(); });

    REQUIRE(shrinkable.value() == 0);
    REQUIRE(shrinkable.value() == 1);
  }

  SECTION("calls shrinks() of the implementation object") {
    auto shrinks = seq::just(shrinkable::just(123, Seq<Shrinkable<int>>()));
    int calledTimes = 0;
    const auto shrinkable = shrinkable::lambda([] { return 0; },
                                               [&] {
                                                 calledTimes++;
                                                 return shrinks;
                                               });

    REQUIRE(calledTimes == 0);
    REQUIRE(shrinkable.shrinks() == shrinks);
    REQUIRE(calledTimes == 1);
    REQUIRE(shrinkable.shrinks() == shrinks);
    REQUIRE(calledTimes == 2);
  }
}

TEST_CASE("shrinkable::just") {
  prop("creates a shrinkable which returns the given value and shrinks",
       [](int value, const Seq<Shrinkable<int>> &shrinks) {
         const auto shrinkable = shrinkable::just(value, shrinks);
         RC_ASSERT(shrinkable.value() == value);
         RC_ASSERT(shrinkable.shrinks() == shrinks);
       });

  SECTION("does not copy on construction if rvalues") {
    const auto shrinkable =
        shrinkable::just(Logger(), seq::just(shrinkable::just(Logger())));
    REQUIRE(shrinkable.value().numberOf("copy") <= 1);
    REQUIRE(shrinkable.shrinks().next()->value().numberOf("copy") <= 1);
  }
}

TEST_CASE("shrinkable::shrink") {
  SECTION("calls the value callable when value() is called each time") {
    int calledTimes = 0;
    const auto shrinkable =
        shrinkable::shrink([&] { return calledTimes++; },
                           [](int) { return Seq<Shrinkable<int>>(); });

    REQUIRE(shrinkable.value() == 0);
    REQUIRE(shrinkable.value() == 1);
  }

  SECTION(
      "calls the shrinks callable with the result of the value callable"
      " each time shrinks() is called") {
    int calledTimes = 0;
    const auto shrinkable = shrinkable::shrink(
        [&] { return calledTimes++; },
        [](int x) { return seq::just(shrinkable::just(x)); });

    REQUIRE(shrinkable.shrinks() == seq::just(shrinkable::just(0)));
    REQUIRE(shrinkable.shrinks() == seq::just(shrinkable::just(1)));
  }
}

TEST_CASE("shrinkable::shrinkRecur") {
  prop("returns a shrinkable with the given value",
       [](int x) {
         const auto shrinkable =
             shrinkable::shrinkRecur(x, [](int) { return Seq<int>(); });
         RC_ASSERT(shrinkable.value() == x);
       });

  prop("recursively applies the shrinking function",
       [] {
         int start = *gen::inRange(0, 100);
         // TODO vector version that takes no generator
         const auto actions =
             *gen::container<std::vector<bool>>(start, gen::arbitrary<bool>());

         const auto shrink = [](int x) {
           return seq::map(seq::range(x, 0), [](int y) { return y - 1; });
         };

         auto shrinkable = shrinkable::shrinkRecur(start, shrink);
         auto shrinks = shrinkable.shrinks();
         int expected = start;
         for (bool branch : actions) {
           RC_ASSERT(shrinkable.value() == expected);
           if (branch) {
             shrinks = shrinkable.shrinks();
           }
           shrinkable = *shrinks.next();
           expected--;
         }

         RC_ASSERT(shrinkable == shrinkable::just(expected));
       });

  SECTION("does not copy value on construction if rvalue") {
    const auto shrinkable = shrinkable::shrinkRecur(
        Logger(), [](const Logger &) { return Seq<Logger>(); });
    REQUIRE(shrinkable.value().numberOf("copy") <= 1);
  }
}
