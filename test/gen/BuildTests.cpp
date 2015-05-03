#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/GenUtils.h"
#include "util/Predictable.h"
#include "util/Logger.h"

using namespace rc;
using namespace rc::test;

template <typename T>
Gen<typename T::element_type> mapDeref(Gen<T> ptrGen) {
  return gen::map(std::move(ptrGen),
                  [](T && value) ->
                  typename T::element_type { return std::move(*value); });
}

TEST_CASE("gen::construct/gen::makeUnique/gen::makeShared") {
  prop("has tuple shrinking semantics",
       [] {
         const auto g1 = genFixedCountdown(*gen::inRange(0, 10));
         const auto g2 = genFixedCountdown(*gen::inRange(0, 10));
         const auto g3 = genFixedCountdown(*gen::inRange(0, 10));

         const auto tupleShrinkable = gen::tuple(g1, g2, g3)(Random(), 0);
         const auto tupleValue = tupleShrinkable.value();
         const auto tupleShrinks =
             shrinkable::immediateShrinks(tupleShrinkable);

         const auto constructShrinkable =
             gen::construct<std::tuple<int, int, int>>(g1, g2, g3)(Random(), 0);

         RC_ASSERT(constructShrinkable.value() == tupleValue);
         RC_ASSERT(shrinkable::immediateShrinks(constructShrinkable) ==
                   tupleShrinks);

         const auto makeUniqueShrinkable =
             mapDeref(gen::makeUnique<std::tuple<int, int, int>>(g1, g2, g3))(
                 Random(), 0);
         RC_ASSERT(makeUniqueShrinkable.value() == tupleValue);
         RC_ASSERT(shrinkable::immediateShrinks(makeUniqueShrinkable) ==
                   tupleShrinks);

         const auto makeSharedShrinkable =
           mapDeref(gen::makeShared<std::tuple<int, int, int>>(g1, g2, g3))(
             Random(), 0);
         RC_ASSERT(makeSharedShrinkable.value() == tupleValue);
         RC_ASSERT(shrinkable::immediateShrinks(makeSharedShrinkable) ==
                   tupleShrinks);
       });
}

TEST_CASE("gen::construct") {
  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::construct<std::tuple<int, int, int>>(
             genSize(), genSize(), genSize());
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(value ==
                   std::make_tuple(params.size, params.size, params.size));
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen = gen::construct<std::tuple<Random, Random, Random>>(
             genRandom(), genRandom(), genRandom());
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(std::get<0>(value) != std::get<1>(value));
         RC_ASSERT(std::get<0>(value) != std::get<2>(value));
         RC_ASSERT(std::get<1>(value) != std::get<2>(value));
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::construct<std::tuple<NonCopyable, NonCopyable>>(
        gen::arbitrary<NonCopyable>(), gen::arbitrary<NonCopyable>());
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }

  SECTION("chooses correct arbitrary instance when not given arguments") {
    const auto gen = gen::construct<std::tuple<Predictable, Predictable>,
                                    Predictable,
                                    Predictable>();
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }
}

TEST_CASE("gen::makeUnique") {
  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::makeUnique<std::tuple<int, int, int>>(
             genSize(), genSize(), genSize());
         const auto value = *gen(params.random, params.size).value();

         RC_ASSERT(value ==
                   std::make_tuple(params.size, params.size, params.size));
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen = gen::makeUnique<std::tuple<Random, Random, Random>>(
             genRandom(), genRandom(), genRandom());
         const auto value = *gen(params.random, params.size).value();

         RC_ASSERT(std::get<0>(value) != std::get<1>(value));
         RC_ASSERT(std::get<0>(value) != std::get<2>(value));
         RC_ASSERT(std::get<1>(value) != std::get<2>(value));
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::makeUnique<std::tuple<NonCopyable, NonCopyable>>(
        gen::arbitrary<NonCopyable>(), gen::arbitrary<NonCopyable>());
    const auto value = std::move(*gen(Random(), 0).value());

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }
}

TEST_CASE("gen::makeShared") {
  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::makeShared<std::tuple<int, int, int>>(
             genSize(), genSize(), genSize());
         const auto value = *gen(params.random, params.size).value();

         RC_ASSERT(value ==
                   std::make_tuple(params.size, params.size, params.size));
       });

  prop("passed random generators are unique",
       [](const GenParams &params) {
         const auto gen = gen::makeShared<std::tuple<Random, Random, Random>>(
             genRandom(), genRandom(), genRandom());
         const auto value = *gen(params.random, params.size).value();

         RC_ASSERT(std::get<0>(value) != std::get<1>(value));
         RC_ASSERT(std::get<0>(value) != std::get<2>(value));
         RC_ASSERT(std::get<1>(value) != std::get<2>(value));
       });

  SECTION("works with non-copyable types") {
    const auto gen = gen::makeShared<std::tuple<NonCopyable, NonCopyable>>(
        gen::arbitrary<NonCopyable>(), gen::arbitrary<NonCopyable>());
    const auto value = std::move(*gen(Random(), 0).value());

    RC_ASSERT(isArbitraryPredictable(std::get<0>(value)));
    RC_ASSERT(isArbitraryPredictable(std::get<1>(value)));
  }
}

namespace {

template <typename T>
struct Foobar {
  Foobar(T x)
      : a(std::move(x)) {}

  void setB(T x) { b = std::move(x); }

  void setCD(T x1, T x2) {
    c = std::move(x1);
    d = std::move(x2);
  }

  std::tuple<T, T, T, T, T> asTuple() const {
    return std::make_tuple(a, b, c, d, e);
  }

  T a;
  T b;
  T c, d;
  T e;
};

} // namespace

TEST_CASE("gen::build") {
  prop("has tuple shrinking semantics",
       [] {
         const auto sizes =
             *gen::container<std::array<int, 5>>(gen::inRange(0, 10));
         std::array<Gen<int>, 5> gens{genFixedCountdown(sizes[0]),
                                      genFixedCountdown(sizes[1]),
                                      genFixedCountdown(sizes[2]),
                                      genFixedCountdown(sizes[3]),
                                      genFixedCountdown(sizes[4])};

         const auto tupleGen =
             gen::tuple(gens[0], gens[1], gens[2], gens[3], gens[4]);
         const auto tupleShrinks =
             shrinkable::immediateShrinks(tupleGen(Random(), 0));

         const auto gen = gen::build(
             gen::construct<Foobar<int>>(gens[0]),
             gen::set(&Foobar<int>::setB, gens[1]),
             gen::set(&Foobar<int>::setCD, gen::tuple(gens[2], gens[3])),
             gen::set(&Foobar<int>::e, gens[4]));
         const auto shrinks =
             seq::map(shrinkable::immediateShrinks(gen(Random(), 0)),
                      [](Foobar<int> &&f) { return f.asTuple(); });

         RC_ASSERT(shrinks == tupleShrinks);
       });

  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::build(
             gen::construct<Foobar<int>>(genSize()),
             gen::set(&Foobar<int>::setB, genSize()),
             gen::set(&Foobar<int>::setCD, gen::tuple(genSize(), genSize())),
             gen::set(&Foobar<int>::e, genSize()));
         const auto value = gen(params.random, params.size).value();

         RC_ASSERT(value.a == params.size);
         RC_ASSERT(value.b == params.size);
         RC_ASSERT(value.c == params.size);
         RC_ASSERT(value.d == params.size);
         RC_ASSERT(value.e == params.size);
       });

  prop("passes unique random generators",
       [](const GenParams &params) {
         const auto gen =
             gen::build(gen::construct<Foobar<Random>>(genRandom()),
                        gen::set(&Foobar<Random>::setB, genRandom()),
                        gen::set(&Foobar<Random>::setCD,
                                 gen::tuple(genRandom(), genRandom())),
                        gen::set(&Foobar<Random>::e, genRandom()));
         const auto value = gen(params.random, params.size).value();

         std::unordered_set<Random> randoms{
             value.a, value.b, value.c, value.d, value.e};
         RC_ASSERT(randoms.size() == 5);
       });

  SECTION("default constructs if generator not present") {
    const auto gen = gen::build<Logger>(gen::set(&Logger::id));
    const auto value = gen(Random(), 0).value();
    REQUIRE(value.log[0] == "default constructed");
  }

  SECTION("works with non-copyable types") {
    const auto gen = gen::build(
        gen::construct<Foobar<NonCopyable>>(gen::arbitrary<NonCopyable>()),
        gen::set(&Foobar<NonCopyable>::setB, gen::arbitrary<NonCopyable>()),
        gen::set(&Foobar<NonCopyable>::setCD,
                 gen::tuple(gen::arbitrary<NonCopyable>(),
                            gen::arbitrary<NonCopyable>())),
        gen::set(&Foobar<NonCopyable>::e, gen::arbitrary<NonCopyable>()));
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(value.a));
    RC_ASSERT(isArbitraryPredictable(value.b));
    RC_ASSERT(isArbitraryPredictable(value.c));
    RC_ASSERT(isArbitraryPredictable(value.d));
    RC_ASSERT(isArbitraryPredictable(value.e));
  }

  SECTION("uses correct arbitrary instances when generator not specified") {
    const auto gen = gen::build(
      gen::construct<Foobar<Predictable>, Predictable>(),
      gen::set(&Foobar<Predictable>::setB),
      gen::set(&Foobar<Predictable>::setCD),
      gen::set(&Foobar<Predictable>::e));
    const auto value = gen(Random(), 0).value();

    RC_ASSERT(isArbitraryPredictable(value.a));
    RC_ASSERT(isArbitraryPredictable(value.b));
    RC_ASSERT(isArbitraryPredictable(value.c));
    RC_ASSERT(isArbitraryPredictable(value.d));
    RC_ASSERT(isArbitraryPredictable(value.e));
  }
}
