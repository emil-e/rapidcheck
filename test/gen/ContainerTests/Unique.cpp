#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "Common.h"

using namespace rc;
using namespace rc::test;

namespace {

struct UniqueFactory {
  template <typename Container, typename T>
  static Gen<Container> makeGen(Gen<T> gen) {
    return gen::unique<Container>(std::move(gen));
  }
};

struct UniqueByFactory {
  template <typename Container, typename T1, typename T2>
  static Gen<Container> makeGen(Gen<std::pair<T1, T2>> gen) {
    return gen::uniqueBy<Container>(
        std::move(gen), [](const std::pair<T1, T2> &p) { return p.first; });
  }
};

template <typename Factory>
struct UniqueByProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("generated values are unique by key",
                     [](const GenParams &params) {
                       const auto gen = Factory::template makeGen<T>(
                           gen::arbitrary<std::pair<int, int>>());
                       onAnyPath(gen(params.random, params.size),
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   const auto v = value.value();
                                   std::set<int> s;
                                   for (const auto &x : v) {
                                     s.insert(x.first);
                                   }
                                   RC_ASSERT(s.size() ==
                                             std::distance(begin(v), end(v)));
                                 });
                     });

    templatedProp<T>(
        "finds minimum where at least two elements must have keys than a "
        "certain value",
        [](const GenParams &params) {
          const auto gen = Factory::template makeGen<T>(
              gen::arbitrary<std::pair<int, int>>());
          const auto target = *gen::inRange(0, 10);
          const auto result = searchGen(
              params.random,
              params.size,
              gen,
              [=](T elements) {
                return std::count_if(begin(elements),
                                     end(elements),
                                     [=](const std::pair<int, int> &x) {
                                       return x.first >= target;
                                     }) >= 2;
              });

          using Set = std::set<std::pair<int, int>>;
          RC_ASSERT((Set(begin(result), end(result)) ==
                     Set{{target + 1, 0}, {target, 0}}));
        });
  }
};

} // namespace

TEST_CASE("gen::uniqueBy") {
  using Pair = std::pair<int, int>;
  forEachType<UniqueByProperties<UniqueByFactory>,
              RC_SEQUENCE_CONTAINERS(Pair)>();
}

namespace {

template <typename Factory>
struct UniqueProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "generated values are unique",
        [](const GenParams &params) {
          const auto gen = Factory::template makeGen<T>(gen::arbitrary<int>());
          onAnyPath(
              gen(params.random, params.size),
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                const auto v = value.value();
                std::set<int> s(begin(v), end(v));
                RC_ASSERT(s.size() == std::distance(begin(v), end(v)));
              });
        });

    templatedProp<T>(
        "finds minimum where at least two elements must have values greater "
        "than a certain value",
        [](const GenParams &params) {
          const auto gen = Factory::template makeGen<T>(gen::arbitrary<int>());
          const auto target = *gen::inRange(0, 10);
          const auto result = searchGen(
              params.random,
              params.size,
              gen,
              [=](T elements) {
                return std::count_if(begin(elements),
                                     end(elements),
                                     [=](int x) { return x >= target; }) >= 2;
              });

          RC_ASSERT((std::set<int>(begin(result), end(result)) ==
                     std::set<int>{target + 1, target}));
        });
  }
};

} // namespace

TEST_CASE("gen::unique") {
  forEachType<GenericProperties<UniqueFactory>,
              RC_SEQUENCE_CONTAINERS(int),
              std::basic_string<int>>();
  forEachType<ParamsProperties<UniqueFactory>,
              RC_SEQUENCE_CONTAINERS(GenParams)>();
  forEachType<UniqueProperties<UniqueFactory>, RC_SEQUENCE_CONTAINERS(int)>();

  forEachType<RetrialProperties<UniqueFactory>,
              RC_SEQUENCE_CONTAINERS(int),
              std::basic_string<int>>();
}
