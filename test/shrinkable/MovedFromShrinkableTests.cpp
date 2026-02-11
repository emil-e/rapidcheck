#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrinkable/Operations.h"
#include "rapidcheck/shrinkable/Transform.h"
#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/seq/Create.h"
#include "rapidcheck/seq/Operations.h"

using namespace rc;

TEST_CASE("Moved-from Shrinkable") {
  // The Shrinkable move constructor sets other.m_impl = nullptr.
  // Without null checks, any subsequent operation on a moved-from Shrinkable
  // will dereference a null pointer and crash.

  SECTION("value() on moved-from Shrinkable must not dereference nullptr") {
    auto original = shrinkable::shrinkRecur(42.0, &shrink::real<double>);
    auto moved = std::move(original);
    // original.m_impl is now nullptr
    // Without the fix, this dereferences nullptr: m_impl->value()
    REQUIRE_THROWS(original.value());
  }

  SECTION("shrinks() on moved-from Shrinkable must not dereference nullptr") {
    auto original = shrinkable::shrinkRecur(42.0, &shrink::real<double>);
    auto moved = std::move(original);
    // Without the fix, this dereferences nullptr: m_impl->shrinks()
    auto s = original.shrinks();
    REQUIRE_FALSE(s.next());
  }

  SECTION(
      "copy of moved-from Shrinkable must not dereference nullptr") {
    auto original = shrinkable::shrinkRecur(42.0, &shrink::real<double>);
    auto moved = std::move(original);
    // Without the fix, this dereferences nullptr: m_impl->retain()
    Shrinkable<double> copy(original);
    (void)copy;
  }

  SECTION("copy assignment from moved-from Shrinkable must not dereference "
          "nullptr") {
    auto original = shrinkable::shrinkRecur(42.0, &shrink::real<double>);
    auto target = shrinkable::shrinkRecur(1.0, &shrink::real<double>);
    auto moved = std::move(original);
    // Without the fix, this dereferences nullptr: other.m_impl->retain()
    target = original;
    (void)target;
  }
}

TEST_CASE("Seq of Shrinkables partially consumed then copied") {
  // This reproduces a real crash path: a custom generator builds a Shrinkable
  // with an explicit shrink sequence (via shrinkable::just with a Seq of
  // shrinks). When the Seq is backed by a JustSeq or ContainerSeq, calling
  // next() MOVES Shrinkable elements out of the internal storage. A subsequent
  // Seq copy (deep-copy via SeqImpl::copy()) copies the underlying container
  // including the moved-from Shrinkable elements, triggering a null-pointer
  // dereference in Shrinkable's copy constructor (m_impl->retain()).

  SECTION("JustSeq with Shrinkable elements: next then copy") {
    // This mimics a custom generator that returns:
    //   shrinkable::just(value, seq::just(shrink1, shrink2))
    auto s = shrinkable::just(
        10.0,
        seq::just(shrinkable::just(5.0), shrinkable::just(0.0)));

    auto shrinks = s.shrinks();
    // Consume one element - moves the Shrinkable out of JustSeq's array
    auto first = shrinks.next();
    REQUIRE(first);
    REQUIRE(first->value() == Approx(5.0));

    // Deep-copying the Seq copies the JustSeq, which copies its std::array,
    // including the moved-from Shrinkable at position 0.
    // Without the fix, this crashes: m_impl->retain() with m_impl == nullptr
    auto shrinksCopy = shrinks;
    auto second = shrinksCopy.next();
    REQUIRE(second);
    REQUIRE(second->value() == Approx(0.0));
  }

  SECTION("ContainerSeq with Shrinkable elements: next then copy") {
    std::vector<Shrinkable<double>> vec;
    vec.push_back(shrinkable::just(5.0));
    vec.push_back(shrinkable::just(3.0));
    vec.push_back(shrinkable::just(0.0));
    auto shrinks = seq::fromContainer(std::move(vec));

    // Consume one element - moves the Shrinkable out of the vector
    auto first = shrinks.next();
    REQUIRE(first);
    REQUIRE(first->value() == Approx(5.0));

    // Deep-copy the partially consumed Seq.
    // ContainerSeq's copy constructor copies the entire vector including the
    // moved-from Shrinkable at index 0.
    // Without the fix, this crashes in Shrinkable's copy constructor.
    auto shrinksCopy = shrinks;
    auto second = shrinksCopy.next();
    REQUIRE(second);
    REQUIRE(second->value() == Approx(3.0));
  }
}

TEST_CASE("Shrinking doubles via findLocalMin does not crash") {
  // Exercise the shrinking search with Shrinkable<double> created via
  // shrinkRecur - the same path used by arbitrary<double>().

  SECTION("plain double Shrinkable") {
    auto s = shrinkable::shrinkRecur(100.5, &shrink::real<double>);
    auto result =
        shrinkable::findLocalMin(s, [](double x) { return x > 5.0; });
    REQUIRE(result.first > 5.0);
  }

  SECTION("mapped double Shrinkable (like gen::map over arbitrary<double>)") {
    // Adds a MapShrinkable layer on top, as gen::map does.
    auto inner = shrinkable::shrinkRecur(100.5, &shrink::real<double>);
    auto mapped =
        shrinkable::map(std::move(inner), [](double d) { return d * 2.0; });
    auto result =
        shrinkable::findLocalMin(mapped, [](double x) { return x > 10.0; });
    REQUIRE(result.first > 10.0);
  }

  SECTION("custom generator style: shrinkable::just with explicit shrinks of "
          "doubles") {
    // A user might write a custom generator that returns a Shrinkable with
    // manually specified shrink candidates. This creates JustSeq-backed
    // Seq<Shrinkable<double>>.
    auto s = shrinkable::just(
        100.0,
        seq::just(shrinkable::shrinkRecur(50.0, &shrink::real<double>),
                  shrinkable::shrinkRecur(10.0, &shrink::real<double>),
                  shrinkable::shrinkRecur(0.0, &shrink::real<double>)));

    auto result =
        shrinkable::findLocalMin(s, [](double x) { return x > 5.0; });
    REQUIRE(result.first > 5.0);
    REQUIRE(result.second > 0);
  }
}
