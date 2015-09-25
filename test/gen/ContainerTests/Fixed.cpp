#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "Common.h"

using namespace rc;
using namespace rc::test;

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
  forEachType<GenericFixedProperties<ContainerFactory>,
              RC_SEQUENCE_CONTAINERS(int),
              RC_SET_CONTAINERS(int),
              std::basic_string<int>>();
  forEachType<GenericFixedProperties<MapFactory>, RC_MAP_CONTAINERS(int)>();

  forEachType<ParamsFixedProperties<ContainerFactory>,
              RC_SEQUENCE_CONTAINERS(GenParams),
              RC_SET_CONTAINERS(GenParams)>();
  forEachType<ParamsFixedProperties<MapFactory>,
              RC_MAP_CONTAINERS(GenParams)>();

  forEachType<RetrialProperties<FixedMapFactory<2, 15>>,
              std::map<int, int>,
              std::unordered_map<int, int>>();
  forEachType<RetrialProperties<FixedContainerFactory<2, 15>>,
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
         } catch (const GenerationFailure &) {
           RC_SUCCEED("Threw GenerationFailure");
         } catch (const std::exception &e) {
           std::cout << e.what() << std::endl;
           RC_FAIL("Threw other exception");
         }
         RC_FAIL("Did not throw GenerationFailure");
       });

  // TODO shrink tests?
}
