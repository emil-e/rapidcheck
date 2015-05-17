#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/gen/Create.h"
#include "rapidcheck/gen/Select.h"

#include "util/Meta.h"
#include "util/Util.h"
#include "util/TypeListMacros.h"
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

struct ElementOfTests {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "all generated elements are elements of the container",
        [](const GenParams &params) {
          T elements = *gen::nonEmpty<T>();
          const auto gen = gen::elementOf(elements);
          const auto value = gen(params.random, params.size).value();
          RC_ASSERT(std::find(begin(elements), end(elements), value) !=
                    end(elements));
        });

    templatedProp<T>("all elements are eventually generated",
                     [](const GenParams &params) {
                       T elements = *gen::nonEmpty<T>();
                       tryUntilAll(std::set<typename T::value_type>(
                                       begin(elements), end(elements)),
                                   gen::elementOf(elements),
                                   params);
                     });

    TEMPLATED_SECTION(T, "throws GenerationFailure on empty container") {
      T container;
      const auto shrinkable = gen::elementOf(container)(Random(0), 0);
      REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
    }
  }
};

} // namespace

TEST_CASE("gen::elementOf") {
  meta::forEachType<ElementOfTests, std::vector<char>, std::string>();
}

TEST_CASE("gen::element") {
  prop("all generated elements are arguments",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         const auto gen = gen::element(a, b, c);
         const auto x = gen(params.random, params.size).value();
         RC_ASSERT((x == a) || (x == b) || (x == c));
       });

  prop("all elements are eventually generated",
       [](const GenParams &params,
          const std::string &a,
          const std::string &b,
          const std::string &c) {
         tryUntilAll({a, b, c}, gen::element(a, b, c), params);
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

  prop("zero weighted elements are never generated",
       [](const GenParams &params) {
         const auto gen =
             gen::weightedElement<int>({{1, 10}, {0, 20}, {3, 30}});
         RC_ASSERT(gen(params.random, params.size).value() != 20);
       });

  SECTION("throws GenerationFailure if weights sum to zero") {
    const auto gen = gen::weightedElement<int>({{0, 1}, {0, 2}, {0, 3}});
    const auto shrinkable = gen(Random(), 0);
    REQUIRE_THROWS_AS(shrinkable.value(), GenerationFailure);
  }
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
