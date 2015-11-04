#include <catch.hpp>
#include <rapidcheck/catch.h>

#include <cctype>

#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/seq/Operations.h"

#include "util/Util.h"
#include "util/Meta.h"
#include "util/TypeListMacros.h"

using namespace rc;
using namespace rc::test;

namespace {

struct RemoveChunksProperties {
  template <typename T>
  static void exec() {
    static const auto fewValues = gen::scale(0.3, gen::arbitrary<T>());
    // TODO non-empty generator
    static const auto fewNonEmptyValues =
        gen::suchThat(fewValues, [](const T &x) { return !x.empty(); });

    templatedProp<T>("first tries empty collection",
                     [] {
                       const auto collection = *fewNonEmptyValues;
                       RC_ASSERT(
                           shrink::removeChunks(collection).next()->empty());
                     });

    templatedProp<T>("successively increases in size for each shrink",
                     [] {
                       const auto seq = shrink::removeChunks(*fewValues);
                       T c;
                       seq::forEach(std::move(seq),
                                    [&](T &&next) {
                                      RC_ASSERT(next.size() >= c.size());
                                      c = std::move(next);
                                    });
                     });

    templatedProp<T>("shrinks to a subset of the original",
                     [] {
                       const auto elements = *fewValues;
                       const auto seq = shrink::removeChunks(elements);
                       seq::forEach(std::move(seq),
                                    [&](T &&c) {
                                      auto diff(
                                          setDifference<char>(c, elements));
                                      RC_ASSERT(diff.size() == 0U);
                                    });
                     });

    templatedProp<T>(
        "every removal of consecutive elements is a possible shrink",
        [] {
          const auto elements = *fewNonEmptyValues;
          const auto size = int(elements.size());
          const auto a = *gen::inRange(0, size + 1);
          const auto b = *gen::distinctFrom(gen::inRange(0, size + 1), a);
          const auto left = std::min(a, b);
          const auto right = std::max(a, b);

          T shrink;
          shrink.reserve(size - (right - left));
          shrink.insert(end(shrink), begin(elements), begin(elements) + left);
          shrink.insert(end(shrink), begin(elements) + right, end(elements));

          RC_ASSERT(seq::contains(shrink::removeChunks(elements), shrink));
        });

    templatedProp<T>(
        "never yields the original value",
        [] {
          auto elements = *fewValues;
          RC_ASSERT(!seq::contains(shrink::removeChunks(elements), elements));
        });
  }
};

} // namespace

TEST_CASE("shrink::removeChunks") {
  forEachType<RemoveChunksProperties, std::vector<char>, std::string>();
}

namespace {

struct EachElementProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "every shrink for every element is tried in order",
        [] {
          const auto elements = *gen::container<T>(gen::nonNegative<char>());
          auto seq = shrink::eachElement(
              elements, [=](char x) { return seq::range(x - 1, -1); });

          for (std::size_t i = 0; i < elements.size(); i++) {
            auto x = elements[i];
            while (x > 0) {
              auto expected = elements;
              expected[i] = --x;
              RC_ASSERT(*seq.next() == expected);
            }
          }

          RC_ASSERT(!seq.next());
        });
  }
};

} // namespace

TEST_CASE("shrink::eachElement") {
  forEachType<EachElementProperties, std::vector<char>, std::string>();
}

namespace {

struct ShrinkTowardsProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("first tries target immediately",
                     [](T target) {
                       T value = *gen::distinctFrom(target);
                       auto seq = shrink::towards(value, target);
                       auto first = seq.next();
                       RC_ASSERT(first);
                       RC_ASSERT(*first == target);
                     });

    templatedProp<T>("tries an adjacent value last",
                     [](T target) {
                       T value = *gen::distinctFrom(target);
                       auto seq = shrink::towards(value, target);
                       auto fin = seq::last(seq);
                       RC_ASSERT(fin);
                       T diff =
                           (value > target) ? (value - *fin) : (*fin - value);
                       RC_ASSERT(diff == T(1));
                     });

    templatedProp<T>(
        "shrinking towards self yields empty shrink",
        [](T target) { RC_ASSERT(!shrink::towards(target, target).next()); });

    templatedProp<T>(
        "never contains original value",
        [](T x, T y) { RC_ASSERT(!seq::contains(shrink::towards(x, y), x)); });
  }
};

} // namespace

TEST_CASE("shrink::towards") {
  forEachType<ShrinkTowardsProperties, RC_INTEGRAL_TYPES>();
}

namespace {

struct IntegralProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("always tries zero first",
                     [] {
                       T value = *gen::nonZero<T>();
                       RC_ASSERT(*shrink::integral<T>(value).next() == T(0));
                     });

    TEMPLATED_SECTION(T, "zero has no shrinks") {
      REQUIRE(!shrink::integral<T>(0).next());
    }

    templatedProp<T>(
        "never contains original value",
        [](T x) { RC_ASSERT(!seq::contains(shrink::integral<T>(x), x)); });
  }
};

struct SignedIntegralProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "shrinks negative values to their positive equivalent",
        [] {
          T value = *gen::suchThat(
                        gen::negative<T>(),
                        [](T x) { return x > std::numeric_limits<T>::min(); });
          RC_ASSERT(seq::contains<T>(shrink::integral<T>(value), -value));
        });

    templatedProp<T>("always tries zero first",
                     [] {
                       T value = *gen::nonZero<T>();
                       RC_ASSERT(*shrink::integral<T>(value).next() == T(0));
                     });

    TEMPLATED_SECTION(T, "zero has no shrinks") {
      REQUIRE(!shrink::integral<T>(0).next());
    }
  }
};

} // namespace

TEST_CASE("shrink::integral") {
  forEachType<IntegralProperties, RC_INTEGRAL_TYPES>();
  forEachType<SignedIntegralProperties, RC_SIGNED_INTEGRAL_TYPES>();
}

namespace {

struct RealProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>(
        "shrinks to nearest integer",
        [] {
          // TODO I'm not super fond of this way of generating
          const auto integer = *gen::arbitrary<int16_t>();
          const auto value =
              static_cast<T>(integer + *gen::element(0.1, 0.2, 0.5, 0.7, 0.9));
          RC_PRE(value != std::trunc(value));
          RC_ASSERT(seq::contains(shrink::real(value), std::trunc(value)));
        });

    TEMPLATED_SECTION(T, "zero has no shrinks") {
      REQUIRE(!shrink::real<T>(0.0).next());
    }

    templatedProp<T>("tries 0.0 first",
                     [] {
                       T value = *gen::nonZero<T>();
                       RC_ASSERT(*shrink::real<T>(value).next() == 0.0);
                     });

    templatedProp<T>(
        "never contains original value",
        [](T x) { RC_ASSERT(!seq::contains(shrink::real<T>(x), x)); });
  }
};

} // namespace

TEST_CASE("shrink::real") { forEachType<RealProperties, RC_REAL_TYPES>(); }

TEST_CASE("shrink::bool") {
  SECTION("shrinks 'true' to 'false'") {
    REQUIRE(shrink::boolean(true) == seq::just(false));
  }

  SECTION("does not shrink 'false'") {
    REQUIRE(!shrink::boolean(false).next());
  }
}

struct CharacterProperties {
  template <typename T>
  static void exec() {
    templatedProp<T>("'a' is the first shrink",
                     [](T x) {
                       RC_PRE(x != 'a');
                       RC_ASSERT(*shrink::character(x).next() == 'a');
                     });

    templatedProp<T>(
        "if uppercase, contains lowercase",
        [] {
          static const std::string letters("ABCDEFGHIJKLMNOPQRSTUVXYZ");
          T x = *gen::elementOf(letters);
          RC_ASSERT(seq::contains<T>(shrink::character(x), std::tolower(x)));
        });

    templatedProp<T>(
        "never contains self",
        [](T x) { RC_ASSERT(!seq::contains(shrink::character(x), x)); });
  }
};

TEST_CASE("shrink::character") {
  forEachType<CharacterProperties, char, wchar_t>();
}
