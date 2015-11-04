#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "util/Predictable.h"

#include "Common.h"

using namespace rc;
using namespace rc::test;

namespace {

struct SetProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "finds minimum where some elements need to be larger than some value",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 50);

          const auto gen = gen::container<T>(gen::arbitrary<int>());

          const int target = *gen::inRange<int>(0, 10);
          const auto result = searchGen(random,
                                        size,
                                        gen,
                                        [=](const T &x) {
                                          int count = 0;
                                          for (const auto &e : x) {
                                            if (e >= target) {
                                              count++;
                                              if (count >= 2) {
                                                return true;
                                              }
                                            }
                                          }

                                          return false;
                                        });

          const T expected{target, target + 1};
          RC_ASSERT(result == expected);
        });
  }
};

struct MultiSetProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "finds minimum where some elements need to be larger than some value",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 50);

          const auto gen = gen::container<T>(gen::arbitrary<int>());

          const int target = *gen::inRange<int>(0, 10);
          const auto result = searchGen(random,
                                        size,
                                        gen,
                                        [=](const T &x) {
                                          int count = 0;
                                          for (const auto &e : x) {
                                            if (e >= target) {
                                              count++;
                                              if (count >= 2) {
                                                return true;
                                              }
                                            }
                                          }

                                          return false;
                                        });

          const T expected{target, target};
          RC_ASSERT(result == expected);
        });
  }
};

struct MapProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "finds minimum where at least two key-value pairs must have values"
        " greater than a certain value",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 50);

          const auto gen =
              gen::container<T>(gen::arbitrary<int>(), gen::arbitrary<int>());

          const int target = *gen::inRange<int>(0, 10);
          const auto result =
              searchGen(random,
                        size,
                        gen,
                        [=](const T &x) {
                          int count = 0;
                          for (const auto &p : x) {
                            if ((p.first >= target) && (p.second >= target)) {
                              count++;
                              if (count >= 2) {
                                return true;
                              }
                            }
                          }

                          return false;
                        });

          const T expected{{target, target}, {target + 1, target}};
          RC_ASSERT(result == expected);
        });
  }
};

struct MultiMapProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "finds minimum where at least two key-value pairs must have values"
        " greater than a certain value",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 50);

          const auto gen =
              gen::container<T>(gen::arbitrary<int>(), gen::arbitrary<int>());

          const int target = *gen::inRange<int>(0, 10);
          const auto result =
              searchGen(random,
                        size,
                        gen,
                        [=](const T &x) {
                          int count = 0;
                          for (const auto &p : x) {
                            if ((p.first >= target) && (p.second >= target)) {
                              count++;
                              if (count >= 2) {
                                return true;
                              }
                            }
                          }

                          return false;
                        });

          const T expected{{target, target}, {target, target}};
          RC_ASSERT(result == expected);
        });
  }
};

struct ArbitraryProperties {
  template <typename T>
  static void exec() {
    using Element = typename T::value_type;

    templatedProp<T>("uses the correct Arbitrary instance",
                     [] {
                       const auto value =
                           gen::arbitrary<T>()(Random(), 0).value();
                       RC_ASSERT(std::all_of(begin(value),
                                             end(value),
                                             [](const Element &x) {
                                               return isArbitraryPredictable(x);
                                             }));
                     });
  }
};

} // namespace

TEST_CASE("gen::container") {
  forEachType<GenericProperties<ContainerFactory>,
              RC_SEQUENCE_CONTAINERS(int),
              RC_SET_CONTAINERS(int),
              std::basic_string<int>>();
  forEachType<GenericProperties<MapFactory>, RC_MAP_CONTAINERS(int)>();

  forEachType<ParamsProperties<ContainerFactory>,
              RC_SEQUENCE_CONTAINERS(GenParams),
              RC_SET_CONTAINERS(GenParams)>();
  forEachType<ParamsProperties<MapFactory>, RC_MAP_CONTAINERS(GenParams)>();

  forEachType<SequenceProperties,
              RC_SEQUENCE_CONTAINERS(int),
              std::basic_string<int>>();

  forEachType<SetProperties, std::set<int>, std::unordered_set<int>>();

  forEachType<MultiSetProperties,
              std::multiset<int>,
              std::unordered_multiset<int>>();

  forEachType<MapProperties,
              std::map<int, int>,
              std::unordered_map<int, int>>();

  forEachType<MultiMapProperties,
              std::multimap<int, int>,
              std::unordered_multimap<int, int>>();

  forEachType<ArbitraryProperties,
              RC_GENERIC_CONTAINERS(Predictable),
              RC_GENERIC_CONTAINERS(NonCopyable),
              std::array<Predictable, 5>,
              std::array<NonCopyable, 5>>();

  forEachType<RetrialProperties<MapFactory>,
              std::map<int, int>,
              std::unordered_map<int, int>>();
  forEachType<RetrialProperties<ContainerFactory>,
              std::set<int>,
              std::unordered_set<int>>();
}
