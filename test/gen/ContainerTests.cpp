#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/Container.h"
#include "rapidcheck/gen/Numeric.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/Meta.h"
#include "util/Util.h"
#include "util/TypeListMacros.h"
#include "util/ArbitraryRandom.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/Predictable.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::test;

namespace {

struct ContainerFactory {
  template <typename Container, typename T>
  static Gen<Container> makeGen(Gen<T> gen) {
    return gen::container<Container>(std::move(gen));
  }

  template <typename Container, typename T>
  static Gen<Container> makeGen(std::size_t count, Gen<T> gen) {
    return gen::container<Container>(count, std::move(gen));
  }
};

template <std::size_t Min, std::size_t Max>
struct FixedContainerFactory {
  template <typename Container, typename T>
  static Gen<Container> makeGen(Gen<T> gen) {
    return gen::container<Container>(*gen::inRange(Min, Max), std::move(gen));
  }
};

struct MapFactory {
  template <typename Map, typename T>
  static Gen<Map> makeGen(Gen<T> gen) {
    return gen::container<Map>(gen, gen);
  }

  template <typename Map, typename T>
  static Gen<Map> makeGen(std::size_t count, Gen<T> gen) {
    return gen::container<Map>(count, gen, gen);
  }
};

template <std::size_t Min, std::size_t Max>
struct FixedMapFactory {
  template <typename Container, typename T>
  static Gen<Container> makeGen(Gen<T> gen) {
    return gen::container<Container>(*gen::inRange(Min, Max), gen, gen);
  }
};

bool hasSize(int size, const std::pair<const GenParams, GenParams> &p) {
  return (p.first.size == size) && (p.second.size == size);
}

bool hasSize(int size, const GenParams &params) { return size == params.size; }

bool insertRandoms(std::unordered_set<Random> &randoms,
                   const std::pair<const GenParams, GenParams> &p) {
  return randoms.insert(p.first.random).second &&
      randoms.insert(p.second.random).second;
}

bool insertRandoms(std::unordered_set<Random> &randoms,
                   const GenParams &params) {
  return randoms.insert(params.random).second;
}

template <typename Factory>
struct GenericProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "generated container never has more elements than the current size",
        [](const GenParams &params) {
          const auto value = Factory::template makeGen<T>(genCountdown())(
                                 params.random, params.size)
                                 .value();
          RC_ASSERT(std::distance(begin(value), end(value)) <= params.size);
        });

    templatedProp<T>("first shrink is empty",
                     [](const GenParams &params) {
                       const auto shrinkable = Factory::template makeGen<T>(
                           genCountdown())(params.random, params.size);
                       RC_PRE(!shrinkable.value().empty());
                       RC_ASSERT(shrinkable.shrinks().next()->value().empty());
                     });

    templatedProp<T>(
        "the size of each shrink is the same or smaller than the original",
        [](const GenParams &params) {
          const auto shrinkable = Factory::template makeGen<T>(genCountdown())(
              params.random, params.size);
          onAnyPath(
              shrinkable,
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(containerSize(shrink.value()) <=
                          containerSize(value.value()));
              });
        });

    templatedProp<T>("none of the shrinks equal the original value",
                     [](const GenParams &params) {
                       const auto shrinkable = Factory::template makeGen<T>(
                           genCountdown())(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() != shrink.value());
                                 });
                     });
  }
};

template <typename Factory>
struct ParamsProperties {
  template <typename T>
  static void exec() {
    using Element = typename T::value_type;

    templatedProp<T>("passes the correct size to the element generators",
                     [](const GenParams &params) {
                       const auto value =
                           Factory::template makeGen<T>(genPassedParams())(
                               params.random, params.size)
                               .value();
                       RC_ASSERT(std::all_of(begin(value),
                                             end(value),
                                             [&](const Element &x) {
                                               return hasSize(params.size, x);
                                             }));
                     });

    templatedProp<T>(
        "the random generators passed to element generators are unique",
        [](const GenParams &params) {
          const auto value = Factory::template makeGen<T>(genPassedParams())(
                                 params.random, params.size)
                                 .value();
          std::unordered_set<Random> randoms;
          RC_ASSERT(std::all_of(
              begin(value),
              end(value),
              [&](const Element &x) { return insertRandoms(randoms, x); }));
        });
  }
};

struct SequenceProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "finds minimum where a particular range of consecutive elements"
        " must be removed at once",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 50);
          const auto shrinkable =
              gen::container<T>(gen::arbitrary<int>())(random, size);
          const auto value = shrinkable.value();

          const auto i1 = *gen::inRange<std::size_t>(0, containerSize(value));
          const auto i2 =
              *gen::distinctFrom(
                  gen::inRange<std::size_t>(0, containerSize(value)), i1);
          // TODO range generator
          const auto il = std::min(i1, i2);
          const auto ir = std::max(i1, i2);
          std::array<int, 2> values{*std::next(begin(value), il),
                                    *std::next(begin(value), ir)};

          const auto pred = [&](const T &x) {
            return std::search(begin(x), end(x), begin(values), end(values)) !=
                end(x);
          };

          const auto result = shrinkable::findLocalMin(shrinkable, pred);
          const T expected{values[0], values[1]};
          RC_ASSERT(result.first == expected);
        });

    templatedProp<T>(
        "finds minimum where some elements need to be larger than some value",
        [](const Random &random) {
          int size = *gen::inRange<int>(0, 50);

          const int target = *gen::inRange<int>(0, 10);
          const auto gen = gen::container<T>(gen::arbitrary<int>());
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
          using Pair = std::pair<const int, int>;

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
          using Pair = std::pair<const int, int>;

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

template <typename Factory>
struct RetrialProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("gives up if not enough unique elements can be generated",
                     [](const Random &random) {
                       const auto gen = Factory::template makeGen<T>(gen::just(0));
                       auto r = random;
                       while (true) {
                         try {
                           gen(r.split(), kNominalSize).value();
                         } catch (const GenerationFailure &) {
                           RC_SUCCEED("Threw GenerationFailure");
                         }
                       }
                     });

    templatedProp<T>(
        "increases size when enough unique elements cannot be generated",
        [](const GenParams &params) {
          RC_PRE(params.size > 0);
          const auto gen = Factory::template makeGen<T>(genSize());
          auto r = params.random;
          try {
            gen(r.split(), params.size).value();
          } catch (const GenerationFailure &e) {
            RC_FAIL(std::string("Threw GenerationFailure: ") + e.what());
          }
        });
  }
};

} // namespace

TEST_CASE("gen::container") {
  meta::forEachType<GenericProperties<ContainerFactory>,
                    RC_SEQUENCE_CONTAINERS(int),
                    RC_SET_CONTAINERS(int),
                    std::basic_string<int>>();
  meta::forEachType<GenericProperties<MapFactory>,
                    RC_MAP_CONTAINERS(int)>();

  meta::forEachType<ParamsProperties<ContainerFactory>,
                    RC_SEQUENCE_CONTAINERS(GenParams),
                    RC_SET_CONTAINERS(GenParams)>();
  meta::forEachType<ParamsProperties<MapFactory>,
                    RC_MAP_CONTAINERS(GenParams)>();

  meta::forEachType<SequenceProperties,
                    RC_SEQUENCE_CONTAINERS(int),
                    std::basic_string<int>>();

  meta::forEachType<SetProperties, std::set<int>, std::unordered_set<int>>();

  meta::forEachType<MultiSetProperties,
                    std::multiset<int>,
                    std::unordered_multiset<int>>();

  meta::forEachType<MapProperties,
                    std::map<int, int>,
                    std::unordered_map<int, int>>();

  meta::forEachType<MultiMapProperties,
                    std::multimap<int, int>,
                    std::unordered_multimap<int, int>>();

  meta::forEachType<ArbitraryProperties,
                    RC_GENERIC_CONTAINERS(Predictable),
                    RC_GENERIC_CONTAINERS(NonCopyable),
                    std::array<Predictable, 5>,
                    std::array<NonCopyable, 5>>();

  meta::forEachType<RetrialProperties<MapFactory>,
                    std::map<int, int>,
                    std::unordered_map<int, int>>();
  meta::forEachType<RetrialProperties<ContainerFactory>,
                    std::set<int>,
                    std::unordered_set<int>>();
}

namespace {

template <typename Factory>
struct GenericFixedProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "generated value always has the requested number of elements",
        [](const GenParams &params) {
          const auto count = *gen::inRange<std::size_t>(0, 10);
          const auto shrinkable = Factory::template makeGen<T>(
              count, genCountdown())(params.random, params.size);
          onAnyPath(
              shrinkable,
              [=](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(containerSize(shrink.value()) == count);
              });
        });

    templatedProp<T>("none of the shrinks equal the original value",
                     [](const GenParams &params) {
                       const auto count = *gen::inRange<std::size_t>(0, 10);
                       const auto shrinkable = Factory::template makeGen<T>(
                           count, genCountdown())(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() != shrink.value());
                                 });
                     });
  }
};

template <typename Factory>
struct ParamsFixedProperties {
  template <typename T>
  static void exec() {
    using Element = typename T::value_type;

    templatedProp<T>("passes the correct size to the element generators",
                     [](const GenParams &params) {
                       const auto count = *gen::inRange<std::size_t>(0, 10);
                       const auto value = Factory::template makeGen<T>(
                                              count, genPassedParams())(
                                              params.random, params.size)
                                              .value();
                       RC_ASSERT(std::all_of(begin(value),
                                             end(value),
                                             [&](const Element &x) {
                                               return hasSize(params.size, x);
                                             }));
                     });

    templatedProp<T>(
        "the random generators passed to element generators are unique",
        [](const GenParams &params) {
          const auto count = *gen::inRange<std::size_t>(0, 10);
          const auto value =
              Factory::template makeGen<T>(count, genPassedParams())(
                  params.random, params.size)
                  .value();
          std::unordered_set<Random> randoms;
          RC_ASSERT(std::all_of(
              begin(value),
              end(value),
              [&](const Element &x) { return insertRandoms(randoms, x); }));
        });
  }
};

} // namespace

TEST_CASE("gen::container(std::size_t)") {
  meta::forEachType<GenericFixedProperties<ContainerFactory>,
                    RC_SEQUENCE_CONTAINERS(int),
                    RC_SET_CONTAINERS(int),
                    std::basic_string<int>>();
  meta::forEachType<GenericFixedProperties<MapFactory>,
                    RC_MAP_CONTAINERS(int)>();

  meta::forEachType<ParamsFixedProperties<ContainerFactory>,
                    RC_SEQUENCE_CONTAINERS(GenParams),
                    RC_SET_CONTAINERS(GenParams)>();
  meta::forEachType<ParamsFixedProperties<MapFactory>,
                    RC_MAP_CONTAINERS(GenParams)>();

  meta::forEachType<RetrialProperties<FixedMapFactory<2, 15>>,
                    std::map<int, int>,
                    std::unordered_map<int, int>>();
  meta::forEachType<RetrialProperties<FixedContainerFactory<2, 15>>,
                    std::set<int>,
                    std::unordered_set<int>>();

  prop("throws GenerationFailure for std::array if count != N",
       [](const GenParams &params) {
         const auto count = *gen::distinctFrom(3);
         const auto gen =
             gen::container<std::array<int, 3>>(count, gen::arbitrary<int>());
         const auto shrinkable = gen(params.random, params.size);
         try {
           shrinkable.value();
         } catch (const GenerationFailure &e) {
           RC_SUCCEED("Threw GenerationFailure");
         } catch (const std::exception &e) {
           std::cout << e.what() << std::endl;
           RC_FAIL("Threw other exception");
         }
         RC_FAIL("Did not throw GenerationFailure");
       });

  // TODO shrink tests?
}

struct UniqueFactory {
  template <typename Container, typename T>
  static Gen<Container> makeGen(Gen<T> gen) {
    return gen::unique<Container>(std::move(gen));
  }
};

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

TEST_CASE("gen::unique") {
  meta::forEachType<GenericProperties<ContainerFactory>,
                    RC_SEQUENCE_CONTAINERS(int),
                    std::basic_string<int>>();
  meta::forEachType<ParamsProperties<ContainerFactory>,
                    RC_SEQUENCE_CONTAINERS(GenParams)>();
  meta::forEachType<UniqueProperties<UniqueFactory>,
                    RC_SEQUENCE_CONTAINERS(int)>();

  meta::forEachType<RetrialProperties<UniqueFactory>,
                    RC_SEQUENCE_CONTAINERS(int),
                    std::basic_string<int>>();
}

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

TEST_CASE("gen::uniqueBy") {
  using Pair = std::pair<int, int>;
  meta::forEachType<UniqueByProperties<UniqueByFactory>,
                    RC_SEQUENCE_CONTAINERS(Pair)>();
}
