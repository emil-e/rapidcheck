#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/Create.h"
#include "rapidcheck/gen/Select.h"

#include "util/Meta.h"
#include "util/Util.h"
#include "util/TypeListMacros.h"
#include "util/ShrinkableUtils.h"
#include "util/GenUtils.h"
#include "util/Predictable.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace {

template <typename T>
double
probabilityOf(const T &value, const Gen<T> &gen, const GenParams &params) {
  auto random = params.random;
  constexpr auto kTotal = 10000;
  auto n = 0;
  for (auto i = 0; i < kTotal; i++) {
    if (gen(random.split(), params.size).value() == value)
      n++;
  }

  return n / static_cast<double>(kTotal);
}

template <typename T>
void tryUntilAll(const std::set<T> &values,
                 const Gen<T> &gen,
                 const GenParams &params) {
  std::set<T> generated;
  auto random = params.random;
  while (values != generated) {
    generated.insert(gen(random.split(), params.size).value());
  }
}

} // namespace

struct ElementOfContainerProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "works with all basic container types",
        [](const GenParams &params) {
          const auto elements = *gen::nonEmpty<T>();
          const auto gen = gen::elementOf(elements);
          const auto value = gen(params.random, params.size).value();
          RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                    end(elements));
        });
  }
};

TEST_CASE("gen::elementOf") {
  prop("all generated elements are elements of the container",
       [](const GenParams &params) {
         const auto elements = *gen::nonEmpty<std::vector<int>>();
         const auto gen = gen::elementOf(elements);
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                   end(elements));
       });

  prop("all elements are eventually generated",
       [](const GenParams &params) {
         const auto elements = *gen::nonEmpty<std::vector<int>>();
         tryUntilAll(std::set<int>(begin(elements), end(elements)),
                     gen::elementOf(elements),
                     params);
       });

  SECTION("throws GenerationFailure on empty container") {
    std::vector<int> container;
    const auto shrinkable = gen::elementOf(container)(Random(0), 0);
    REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
  }

  forEachType<ElementOfContainerProperties,
              RC_GENERIC_CONTAINERS(int),
              std::array<int, 5>,
              std::string>();
}

TEST_CASE("gen::element") {
  prop("equivalent to gen::elementOf",
       [](const GenParams &params, int a, int b, int c) {
         const auto expectedShrinkable = gen::elementOf(
             std::vector<int>{a, b, c})(params.random, params.size);
         const auto shrinkable =
             gen::element(a, b, c)(params.random, params.size);
         assertEquivalent(expectedShrinkable, shrinkable);
       });
}

TEST_CASE("gen::weightedElement") {
  prop("respects weights",
       [](const GenParams &params) {
         const auto p = probabilityOf(
             0,
             gen::weightedElement<int>({{1, 10}, {1, 20}, {12, 0}, {2, 1}}),
             params);
         RC_ASSERT(std::abs(p - 0.75) < 0.03);
       });

  prop("all generated elements are arguments",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         const auto gen =
             gen::weightedElement<std::string>({{1, a}, {2, b}, {3, c}});
         const auto x = gen(params.random, params.size).value();
         RC_ASSERT((x == a) || (x == b) || (x == c));
       });

  prop("all non-zero-weighted elements are eventually generated",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         tryUntilAll(
             {a, b, c},
             gen::weightedElement<std::string>({{1, a}, {2, b}, {3, c}}),
             params);
       });

  prop(
      "zero weighted elements are never generated",
      [](const GenParams &params) {
        const auto gen = gen::weightedElement<int>({{1, 10}, {0, 20}, {3, 30}});
        RC_ASSERT(gen(params.random, params.size).value() != 20);
      });

  SECTION("throws GenerationFailure if weights sum to zero") {
    const auto gen = gen::weightedElement<int>({{0, 1}, {0, 2}, {0, 3}});
    const auto shrinkable = gen(Random(), 0);
    REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
  }
}

namespace {

struct SizedElementOfContainerProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "works with all basic container types",
        [](const GenParams &params) {
          const auto elements = *gen::nonEmpty<T>();
          const auto gen = gen::sizedElementOf(elements);
          const auto value = gen(params.random, params.size).value();
          RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                    end(elements));
        });
  }
};

} // namespace

TEST_CASE("gen::sizedElementOf") {
  SECTION("when size is zero") {
    prop("generates only the first element",
         [](const Random &random) {
           const auto elements = *gen::nonEmpty<std::vector<int>>();
           const auto gen = gen::sizedElementOf(elements);
           const auto value = gen(random, 0).value();
           RC_ASSERT(value == elements.front());
         });
  }

  SECTION("when size is kNominalSize") {
    prop("all elements are eventually generated",
         [](const Random &random) {
           const auto elements = *gen::nonEmpty<std::vector<int>>();
           tryUntilAll(std::set<int>(begin(elements), end(elements)),
                       gen::sizedElementOf(elements),
                       GenParams(random, kNominalSize));
         });
  }

  prop("all generated values are elements",
       [](const GenParams &params) {
         const auto elements = *gen::nonEmpty<std::vector<int>>();
         const auto gen = gen::sizedElementOf(elements);
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                   end(elements));
       });

  prop("first shrink is always the first element",
       [](const GenParams &params) {
         const auto elements = *gen::nonEmpty<std::vector<int>>();
         const auto gen = gen::sizedElementOf(elements);
         onAnyPath(
             gen(params.random, params.size),
             [&](const Shrinkable<int> &value, const Shrinkable<int> &shrink) {
               if (value.value() != elements.front()) {
                 RC_ASSERT(value.shrinks().next()->value() == elements.front());
               }
             });
       });

  prop("finds minimum where must be larger than one of the elements",
       [](const Random &random) {
         const auto target = *gen::inRange(0, 10);
         const auto result =
             searchGen(random,
                       kNominalSize,
                       gen::sizedElementOf(std::string("0123456789")),
                       [=](char c) { return (c - '0') >= target; });
         RC_ASSERT((result - '0') == target);
       });

  SECTION("throws GenerationFailure on empty container") {
    std::vector<int> container;
    const auto shrinkable = gen::sizedElementOf(container)(Random(0), 0);
    REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
  }

  forEachType<SizedElementOfContainerProperties,
              RC_GENERIC_CONTAINERS(int),
              std::array<int, 5>,
              std::string>();
}

TEST_CASE("gen::sizedElement") {
  prop("equivalent to gen::sizedElementOf",
       [](const GenParams &params, int a, int b, int c) {
         const auto expectedShrinkable = gen::sizedElementOf(
             std::vector<int>{a, b, c})(params.random, params.size);
         const auto shrinkable =
             gen::sizedElement(a, b, c)(params.random, params.size);
         assertEquivalent(expectedShrinkable, shrinkable);
       });
}

TEST_CASE("gen::oneOf") {
  prop("all generated elements come from one of the generators",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         const auto gen = gen::oneOf(gen::just(a), gen::just(b), gen::just(c));
         const auto x = gen(params.random, params.size).value();
         RC_ASSERT((x == a) || (x == b) || (x == c));
       });

  prop("all generators are eventually used",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         tryUntilAll({a, b, c},
                     gen::oneOf(gen::just(a), gen::just(b), gen::just(c)),
                     params);
       });

  prop("passes the correct size to the generators",
       [](const GenParams &params) {
         const auto gen = gen::oneOf(genPassedParams());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value.size == params.size);
       });

  prop("works with non-copyable types",
       [](const GenParams &params) {
         const auto gen = gen::oneOf(gen::arbitrary<NonCopyable>());
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(isArbitraryPredictable(value));
       });
}

TEST_CASE("gen::weightedOneOf") {
  prop("respects weights",
       [](const GenParams &params) {
         const auto p =
             probabilityOf(0,
                           gen::weightedOneOf<int>({{1, gen::just(10)},
                                                    {1, gen::just(20)},
                                                    {12, gen::just(0)},
                                                    {2, gen::just(1)}}),
                           params);
         RC_ASSERT(std::abs(p - 0.75) < 0.03);
       });

  prop("uses one of the given generators",
       [](const GenParams &params,
          const Shrinkable<int> &a,
          const Shrinkable<int> &b,
          const Shrinkable<int> &c) {
         const auto gen = gen::weightedOneOf<int>({{1, fn::constant(a)},
                                                   {2, fn::constant(b)},
                                                   {3, fn::constant(c)}});
         const auto shrinkable = gen(params.random, params.size);
         RC_ASSERT((shrinkable == a) || (shrinkable == b) || (shrinkable == c));
       });

  prop("passes correct size",
       [](const GenParams &params) {
         const auto gen = gen::weightedOneOf<int>({{1, genSize()}});
         const auto value = gen(params.random, params.size).value();
         RC_ASSERT(value == params.size);
       });

  prop("all non-zero-weighted elements are eventually generated",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         tryUntilAll(
             {a, b, c},
             gen::weightedOneOf<std::string>(
                 {{1, gen::just(a)}, {2, gen::just(b)}, {3, gen::just(c)}}),
             params);
       });

  prop("zero weighted elements are never generated",
       [](const GenParams &params) {
         const auto gen = gen::weightedOneOf<int>(
             {{1, gen::just(10)}, {0, gen::just(20)}, {3, gen::just(30)}});
         RC_ASSERT(gen(params.random, params.size).value() != 20);
       });

  SECTION("throws GenerationFailure if weights sum to zero") {
    const auto gen = gen::weightedOneOf<int>(
        {{0, gen::just(1)}, {0, gen::just(2)}, {0, gen::just(3)}});
    const auto shrinkable = gen(Random(), 0);
    REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
  }
}

namespace {

template <typename T>
Gen<std::pair<T, int>> genTagged(Gen<T> gen, int tag) {
  return gen::map(std::move(gen),
                  [=](T &&x) { return std::make_pair(std::move(x), tag); });
}

} // namespace

TEST_CASE("gen::sizedOneOf") {
  SECTION("when size is zero") {
    prop("uses only the first generator",
         [](const Random &random, int a, int b, int c) {
           const auto gen =
               gen::sizedOneOf(gen::just(a), gen::just(b), gen::just(c));
           const auto value = gen(random, 0).value();
           RC_ASSERT(value == a);
         });
  }

  SECTION("when size is kNominalSize") {
    prop("all generator are eventually used",
         [](const Random &random, int a, int b, int c) {
           const auto elements = *gen::nonEmpty<std::vector<int>>();
           tryUntilAll(
               std::set<int>{a, b, c},
               gen::sizedOneOf(gen::just(a), gen::just(b), gen::just(c)),
               GenParams(random, kNominalSize));
         });
  }

  prop("only uses provided generators",
       [](const GenParams &params, int a, int b, int c) {
         const auto gen =
             gen::sizedOneOf(gen::just(a), gen::just(b), gen::just(c));
         const auto x = gen(params.random, params.size).value();
         RC_ASSERT((x == a) || (x == b) || (x == c));
       });

  prop("first shrink is using first generator",
       [](const GenParams &params) {
         const auto gen = gen::sizedOneOf(genTagged(gen::just(0), 0),
                                          genTagged(gen::arbitrary<int>(), 1),
                                          genTagged(gen::arbitrary<int>(), 2));
         const auto shrinkable = gen(params.random, params.size);
         const auto value = shrinkable.value();
         if (value.second != 0) {
           RC_ASSERT(shrinkable.shrinks().next()->value().second == 0);
         }
       });

  prop(
      "after trying to shrink which generator is used, tries to shrink "
      "original value",
      [](const GenParams &params) {
        const auto gen = gen::sizedOneOf(
            genTagged(genFixedCountdown(*gen::inRange(0, 5)), 0),
            genTagged(genFixedCountdown(*gen::inRange(0, 5)), 1),
            genTagged(genFixedCountdown(*gen::inRange(0, 5)), 2));
        const auto shrinkable = gen(params.random, params.size);
        const auto value = shrinkable.value();
        const auto expected = shrinkable::map(
            countdownShrinkable(value.first),
            [=](int x) { return std::make_pair(x, value.second); });
        const auto actual = shrinkable::mapShrinks(
            shrinkable,
            [=](Seq<Shrinkable<std::pair<int, int>>> &&shrinks) {
              return seq::dropWhile(
                  std::move(shrinks),
                  [=](const Shrinkable<std::pair<int, int>> &s) {
                    return s.value().second != value.second;
                  });
            });

        RC_ASSERT(actual == expected);
      });

  prop(
      "finds minimum where tag must be greater than a certain value but value "
      "part does not matter",
      [](const GenParams &params) {
        const auto gen = gen::sizedOneOf(genTagged(gen::arbitrary<int>(), 0),
                                         genTagged(gen::arbitrary<int>(), 1),
                                         genTagged(gen::arbitrary<int>(), 2),
                                         genTagged(gen::arbitrary<int>(), 3),
                                         genTagged(gen::arbitrary<int>(), 4),
                                         genTagged(gen::arbitrary<int>(), 5));
        const auto targetTag = *gen::inRange(0, 6);
        const auto result = searchGen(params.random,
                                      params.size,
                                      gen,
                                      [=](const std::pair<int, int> &p) {
                                        return p.second >= targetTag;
                                      });
        RC_ASSERT(result == std::make_pair(0, targetTag));
      });
}
