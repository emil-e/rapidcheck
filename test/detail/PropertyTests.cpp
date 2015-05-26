#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/Property.h"

#include "util/Generators.h"
#include "util/Predictable.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace {

template <typename Callable>
PropertyWrapper<Decay<Callable>> makeWrapper(Callable &&callable) {
  return PropertyWrapper<Decay<Callable>>(std::forward<Callable>(callable));
}

bool descriptionContains(const WrapperResult &result, const std::string &str) {
  return result.result.description.find(str) != std::string::npos;
}

void reportAll(const std::vector<CaseResult> &results) {
  for (const auto &result : results) {
    ImplicitParam<param::CurrentPropertyContext>::value()->reportResult(result);
  }
}

Gen<CaseResult>
genResultOfType(std::initializer_list<CaseResult::Type> types) {
  return gen::build<CaseResult>(
      gen::set(&CaseResult::type,
               gen::elementOf<std::vector<CaseResult::Type>>(types)),
      gen::set(&CaseResult::description));
}

} // namespace

TEST_CASE("PropertyWrapper") {
  prop("Discard overrides any other reported result",
       [] {
         auto results =
             *gen::container<std::vector<CaseResult>>(genResultOfType(
                 {CaseResult::Type::Success, CaseResult::Type::Failure}));
         const auto discardResult =
             *genResultOfType({CaseResult::Type::Discard});
         const auto position = *gen::inRange<std::size_t>(0, results.size());
         results.insert(begin(results) + position, discardResult);

         const auto result = makeWrapper([=] { reportAll(results); })().result;
         RC_ASSERT(result == discardResult);
       });

  prop("Failure overrides any reported successes",
       [] {
         auto results = *gen::container<std::vector<CaseResult>>(
                            genResultOfType({CaseResult::Type::Success}));
         const auto failureResult =
             *genResultOfType({CaseResult::Type::Failure});
         const auto position = *gen::inRange<std::size_t>(0, results.size());
         results.insert(begin(results) + position, failureResult);

         const auto result = makeWrapper([=] { reportAll(results); })().result;
         RC_ASSERT(result == failureResult);
       });

  prop("result description contains descriptions of all reported failures",
       [] {
         auto failureResults =
             *gen::container<std::vector<CaseResult>>(
                 genResultOfType({CaseResult::Type::Failure}));

         const auto result = makeWrapper([=] { reportAll(failureResults); })();
         for (const auto &failureResult : failureResults) {
           RC_ASSERT(descriptionContains(result, failureResult.description));
         }
       });

  prop("returns CaseResult as is",
       [](const CaseResult &result) {
         RC_ASSERT(makeWrapper([=] { return result; })().result == result);
       });

  SECTION("returns success result for void callables") {
    REQUIRE(makeWrapper([] {})().result.type == CaseResult::Type::Success);
  }

  SECTION("if callable returns a bool") {
    SECTION("returns success result for true") {
      REQUIRE(makeWrapper([] { return true; })().result.type ==
              CaseResult::Type::Success);
    }

    SECTION("returns success result for false") {
      REQUIRE(makeWrapper([] { return false; })().result.type ==
              CaseResult::Type::Failure);
    }
  }

  SECTION("returns success for empty strings") {
    REQUIRE(makeWrapper([] { return std::string(); })().result.type ==
            CaseResult::Type::Success);
  }

  prop("returns failure with the string as message for non-empty strings",
       [] {
         // TODO non-empty generator
         const auto msg = *gen::nonEmpty<std::string>();
         const auto result = makeWrapper([&] { return msg; })();
         RC_ASSERT(result.result.type == CaseResult::Type::Failure);
         RC_ASSERT(descriptionContains(result, msg));
       });

  prop("if a CaseResult is thrown, returns that case result",
       [](const CaseResult &result) {
         RC_ASSERT(makeWrapper([&] { throw result; })().result == result);
       });

  prop("returns a discard result if a GenerationFailure is thrown",
       [](const std::string &msg) {
         const auto result =
             makeWrapper([&] { throw GenerationFailure(msg); })();
         RC_ASSERT(result.result.type == CaseResult::Type::Discard);
         RC_ASSERT(descriptionContains(result, msg));
       });

  prop(
      "returns a failure result with what message if an std::exception is"
      " thrown",
      [](const std::string &msg) {
        const auto result =
            makeWrapper([&] { throw std::runtime_error(msg); })();
        RC_ASSERT(result.result.type == CaseResult::Type::Failure);
        RC_ASSERT(descriptionContains(result, msg));
      });

  prop(
      "returns a failure result with the string as the message if a string"
      " is thrown",
      [](const std::string &msg) {
        const auto result = makeWrapper([&] { throw msg; })();
        RC_ASSERT(result.result.type == CaseResult::Type::Failure);
        RC_ASSERT(descriptionContains(result, msg));
      });

  SECTION("returns a failure result if other values are thrown") {
    const auto result = makeWrapper([&] { throw 1337; })();
    RC_ASSERT(result.result.type == CaseResult::Type::Failure);
  }

  prop("forwards arguments to callable",
       [](int a, const std::string &b, NonCopyable c) {
         const auto expected = std::to_string(a) + b + std::to_string(c.extra);
         const auto wrapper =
             makeWrapper([](int a, const std::string &b, NonCopyable c) {
               return std::to_string(a) + b + std::to_string(c.extra);
             });
         const auto result = wrapper(std::move(a), std::move(b), std::move(c));
         RC_ASSERT(result.result.description == expected);
       });

  prop("returns any tags that were added",
       [](const std::vector<std::string> &tags) {
         const auto result = makeWrapper([&]{
           for (const auto &tag : tags) {
             ImplicitParam<param::CurrentPropertyContext>::value()->addTag(tag);
           }
         })();

         RC_ASSERT(result.tags == tags);
       });
}

namespace {

template <int N>
struct Fixed {};

template <int N>
void showValue(std::ostream &os, const Fixed<N> &) {
  os << N;
}

} // namespace

namespace rc {

template <int N>
struct Arbitrary<Fixed<N>> {
  static Gen<Fixed<N>> arbitrary() {
    return [](const Random &random, int) {
      const int n = Random(random).next() % 10;
      return shrinkable::just(
          Fixed<N>(), seq::take(n, seq::repeat(shrinkable::just(Fixed<N>()))));
    };
  }
};

} // namespace rc

TEST_CASE("toProperty") {
  using ShrinkableResult = Shrinkable<CaseDescription>;

  prop("counterexample contains descriptions of picked values",
       [](const GenParams &params) {
         const auto n = *gen::inRange<std::size_t>(0, 10);
         const auto gen = toProperty([=](Fixed<1>, Fixed<2>, Fixed<3>) {
           for (std::size_t i = 0; i < n; i++) {
             *gen::arbitrary<Fixed<1337>>();
           }
         });
         const auto shrinkable = gen(params.random, params.size);

         Example expected;
         expected.reserve(n + 1);
         using ArgsTuple = std::tuple<Fixed<1>, Fixed<2>, Fixed<3>>;
         expected.emplace_back(
             typeToString<ArgsTuple>(),
             toString(std::make_tuple(Fixed<1>(), Fixed<2>(), Fixed<3>())));
         expected.insert(end(expected),
                         n,
                         std::make_pair(typeToString<Fixed<1337>>(),
                                        toString(Fixed<1337>())));

         onAnyPath(shrinkable,
                   [&](const ShrinkableResult &value,
                       const ShrinkableResult &shrink) {
                     RC_ASSERT(value.value().example == expected);
                   });
       });

  prop(
      "throws in counterexample is replaced with placeholders dscribing the"
      " error",
      [](const GenParams &params, const std::string &msg) {
        const auto n = *gen::inRange<std::size_t>(1, 10);
        const auto throwIndex = *gen::inRange<std::size_t>(0, n);
        const auto gen = toProperty([=] {
          for (std::size_t i = 0; i < n; i++) {
            if (i == throwIndex) {
              // TODO maybe a "throws" generator?
              try {
                *Gen<int>([=](const Random &, int) -> Shrinkable<int> {
                  throw GenerationFailure(msg);
                });
              } catch (...) {
              }
            } else {
              *gen::arbitrary<Fixed<1337>>();
            }
          }
        });
        const auto shrinkable = gen(params.random, params.size);

        Example expected;
        expected.reserve(n);
        expected.insert(end(expected),
                        n,
                        std::make_pair(typeToString<Fixed<1337>>(),
                                       toString(Fixed<1337>())));
        // TODO better test
        expected[throwIndex] = {"Generation failed", msg};

        onAnyPath(
            shrinkable,
            [&](const ShrinkableResult &value, const ShrinkableResult &shrink) {
              RC_ASSERT(value.value().example == expected);
            });
      });

  prop("case result corresponds to counterexample",
       [](const GenParams &params) {
         const auto gen =
             toProperty([=] { return (*gen::arbitrary<int>() % 2) == 0; });
         const auto shrinkable = gen(params.random, params.size);

         onAnyPath(
             shrinkable,
             [](const ShrinkableResult &value, const ShrinkableResult &shrink) {
               const auto desc = value.value();
               RC_ASSERT((desc.result.type == CaseResult::Type::Success) ==
                         ((std::stoi(desc.example.back().second) % 2) == 0));
             });
       });

  prop("tags correspond to counterexample",
       [](const GenParams &params) {
         const auto gen = toProperty([=] {
           const auto tags =
               *gen::scale(0.25, gen::arbitrary<std::vector<std::string>>());
           for (const auto &tag : tags) {
             ImplicitParam<param::CurrentPropertyContext>::value()->addTag(tag);
           }
         });
         const auto shrinkable = gen(params.random, params.size);

         onAnyPath(
             shrinkable,
             [](const ShrinkableResult &value, const ShrinkableResult &shrink) {
               const auto desc = value.value();
               RC_ASSERT(toString(desc.tags) == desc.example.back().second);
             });
       });
}
