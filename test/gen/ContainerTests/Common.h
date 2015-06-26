#pragma once

#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"
#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"

namespace rc {
namespace test {

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

inline bool hasSize(int size, const std::pair<const GenParams, GenParams> &p) {
  return (p.first.size == size) && (p.second.size == size);
}

inline bool hasSize(int size, const GenParams &params) { return size == params.size; }

inline bool insertRandoms(std::unordered_set<Random> &randoms,
                   const std::pair<const GenParams, GenParams> &p) {
  return randoms.insert(p.first.random).second &&
      randoms.insert(p.second.random).second;
}

inline bool insertRandoms(std::unordered_set<Random> &randoms,
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

} // namespace test
} // namespace rc
