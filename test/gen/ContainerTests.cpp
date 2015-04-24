#include <catch.hpp>
#include <rapidcheck-catch.h>

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

template <typename Container,
          bool = rc::gen::detail::IsMapContainer<Container>::value>
struct Generators {
  template <typename T>
  static Gen<Container> make(Gen<T> gen) {
    return gen::container<Container>(std::move(gen));
  }

  template <typename T>
  static Gen<Container> make(std::size_t count, Gen<T> gen) {
    return gen::container<Container>(count, std::move(gen));
  }
};

template <typename Container>
struct Generators<Container, true> {
  template <typename T>
  static Gen<Container> make(Gen<T> gen) {
    return gen::container<Container>(gen, gen);
  }

  template <typename T>
  static Gen<Container> make(std::size_t count, Gen<T> gen) {
    return gen::container<Container>(count, gen, gen);
  }
};

template <typename Container, typename... Args>
Gen<Container> makeGen(Args &&... args) {
  return Generators<Container>::make(std::forward<Args>(args)...);
}

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

struct GenericProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "generated container never has more elements than the current size",
        [](const GenParams &params) {
          const auto value =
              makeGen<T>(genCountdown())(params.random, params.size).value();
          RC_ASSERT(std::distance(begin(value), end(value)) <= params.size);
        });

    templatedProp<T>("first shrink is empty",
                     [](const GenParams &params) {
                       const auto shrinkable = makeGen<T>(genCountdown())(
                           params.random, params.size);
                       RC_PRE(!shrinkable.value().empty());
                       RC_ASSERT(shrinkable.shrinks().next()->value().empty());
                     });

    templatedProp<T>(
        "the size of each shrink is the same or smaller than the original",
        [](const GenParams &params) {
          const auto shrinkable =
              makeGen<T>(genCountdown())(params.random, params.size);
          onAnyPath(
              shrinkable,
              [](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(containerSize(shrink.value()) <=
                          containerSize(value.value()));
              });
        });

    templatedProp<T>("none of the shrinks equal the original value",
                     [](const GenParams &params) {
                       const auto shrinkable = makeGen<T>(genCountdown())(
                           params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() != shrink.value());
                                 });
                     });
  }
};

struct ParamsProperties {
  template <typename T>
  static void exec() {
    using Element = typename T::value_type;

    templatedProp<T>(
        "passes the correct size to the element generators",
        [](const GenParams &params) {
          const auto value =
              makeGen<T>(genPassedParams())(params.random, params.size).value();
          RC_ASSERT(std::all_of(
              begin(value),
              end(value),
              [&](const Element &x) { return hasSize(params.size, x); }));
        });

    templatedProp<T>(
        "the random generators passed to element generators are unique",
        [](const GenParams &params) {
          const auto value =
              makeGen<T>(genPassedParams())(params.random, params.size).value();
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
                                              if (count >= 2)
                                                return true;
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
                                              if (count >= 2)
                                                return true;
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
                                              if (count >= 2)
                                                return true;
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
                              if (count >= 2)
                                return true;
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
                              if (count >= 2)
                                return true;
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
  meta::forEachType<GenericProperties,
                    RC_GENERIC_CONTAINERS(int),
                    std::basic_string<int>>();

  meta::forEachType<ParamsProperties,
                    RC_GENERIC_CONTAINERS(GenParams),
                    std::array<GenParams, 5>>();

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
}

namespace {

struct GenericFixedProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "generated value always has the requested number of elements",
        [](const GenParams &params) {
          const auto count = *gen::inRange<std::size_t>(0, 10);
          const auto shrinkable =
              makeGen<T>(count, genCountdown())(params.random, params.size);
          onAnyPath(
              shrinkable,
              [=](const Shrinkable<T> &value, const Shrinkable<T> &shrink) {
                RC_ASSERT(containerSize(shrink.value()) == count);
              });
        });

    templatedProp<T>("none of the shrinks equal the original value",
                     [](const GenParams &params) {
                       const auto count = *gen::inRange<std::size_t>(0, 10);
                       const auto shrinkable = makeGen<T>(
                           count, genCountdown())(params.random, params.size);
                       onAnyPath(shrinkable,
                                 [](const Shrinkable<T> &value,
                                    const Shrinkable<T> &shrink) {
                                   RC_ASSERT(value.value() != shrink.value());
                                 });
                     });
  }
};

struct ParamsFixedProperties {
  template <typename T>
  static void exec() {
    using Element = typename T::value_type;

    templatedProp<T>("passes the correct size to the element generators",
                     [](const GenParams &params) {
                       const auto count = *gen::inRange<std::size_t>(0, 10);
                       const auto value = makeGen<T>(count, genPassedParams())(
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
              makeGen<T>(count, genPassedParams())(params.random, params.size)
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
  meta::forEachType<GenericFixedProperties,
                    RC_GENERIC_CONTAINERS(int),
                    std::basic_string<int>>();

  meta::forEachType<ParamsFixedProperties, RC_GENERIC_CONTAINERS(GenParams)>();

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
