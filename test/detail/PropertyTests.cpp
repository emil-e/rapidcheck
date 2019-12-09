#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/detail/Property.h"

#include "util/Generators.h"
#include "util/Predictable.h"
#include "util/GenUtils.h"
#include "util/TemplateProps.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

TEST_CASE("CaseDescription") {
  SECTION("operator==/operator!=") {
    propConformsToEquals<CaseDescription>();
    PROP_REPLACE_MEMBER_INEQUAL(CaseDescription, result);
    PROP_REPLACE_MEMBER_INEQUAL(CaseDescription, tags);

    prop("not equal if example not equal",
         [](const CaseDescription &original) {
           auto other(original);
           const auto otherExample = *gen::distinctFrom(other.example());
           other.example = [=] { return otherExample; };
           RC_ASSERT(original != other);
           RC_ASSERT(other != original);
           RC_ASSERT(!(original == other));
           RC_ASSERT(!(other == original));
         });

    SECTION("equal if neither has example") {
      CaseDescription a;
      CaseDescription b;
      REQUIRE(a == b);
    }

    SECTION("inequal if one has example but not the other") {
      CaseDescription a;
      CaseDescription b;
      b.example = [] { return Example(); };
      REQUIRE_FALSE(a == b);
    }
  }

  SECTION("operator<<") {
    propConformsToOutputOperator<CaseDescription>();

    SECTION("prints description without example") {
      std::ostringstream os;
      CaseDescription desc;
      os << desc;
      REQUIRE(os.str().find("example=") == std::string::npos);
    }
  }
}

namespace {

template <typename Callable>
PropertyAdapter<Decay<Callable>> makeAdapter(Callable &&callable) {
  return PropertyAdapter<Decay<Callable>>(std::forward<Callable>(callable));
}

bool descriptionContains(const TaggedResult &result, const std::string &str) {
  return result.result.description.find(str) != std::string::npos;
}

void reportAll(const std::vector<CaseResult> &results) {
  for (const auto &result : results) {
    ImplicitParam<param::CurrentPropertyContext>::value()->reportResult(result);
  }
}

void logAll(const std::vector<std::string> &log) {
  auto &logStream =
      ImplicitParam<param::CurrentPropertyContext>::value()->logStream();
  for (const auto &message : log) {
    logStream << message;
  }
}

Gen<CaseResult> genResultOfType(std::initializer_list<CaseResult::Type> types) {
  return gen::build<CaseResult>(
      gen::set(&CaseResult::type,
               gen::elementOf<std::vector<CaseResult::Type>>(types)),
      gen::set(&CaseResult::description));
}

} // namespace

TEST_CASE("PropertyAdapter") {
  prop("Discard overrides any other reported result",
       [] {
         auto results =
             *gen::container<std::vector<CaseResult>>(genResultOfType(
                 {CaseResult::Type::Success, CaseResult::Type::Failure}));
         const auto discardResult =
             *genResultOfType({CaseResult::Type::Discard});
         const auto position = *gen::inRange<std::size_t>(0, results.size());
         results.insert(begin(results) + position, discardResult);

         const auto result = makeAdapter([=] { reportAll(results); })().result;
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

         const auto result = makeAdapter([=] { reportAll(results); })().result;
         RC_ASSERT(result == failureResult);
       });

  prop("result description contains descriptions of all reported failures",
       [] {
         auto failureResults =
             *gen::container<std::vector<CaseResult>>(
                 genResultOfType({CaseResult::Type::Failure}));

         const auto result = makeAdapter([=] { reportAll(failureResults); })();
         for (const auto &failureResult : failureResults) {
           RC_ASSERT(descriptionContains(result, failureResult.description));
         }
       });

  prop("result description contains all logged messages",
       [](const std::vector<std::string> &messages) {
         const auto result = makeAdapter([=] { logAll(messages); })();
         for (const auto &message : messages) {
           RC_ASSERT(descriptionContains(result, message));
         }
       });

  SECTION("does not include log when nothing was logged") {
    const auto result = makeAdapter([=] {})();
    REQUIRE(!descriptionContains(result, "Log:"));
  }

  prop("returns CaseResult as is",
       [](const CaseResult &result) {
         RC_ASSERT(makeAdapter([=] { return result; })().result == result);
       });

  SECTION("returns success result for void callables") {
    REQUIRE(makeAdapter([] {})().result.type == CaseResult::Type::Success);
  }

  SECTION("if callable returns a bool") {
    SECTION("returns success result for true") {
      REQUIRE(makeAdapter([] { return true; })().result.type ==
              CaseResult::Type::Success);
    }

    SECTION("returns success result for false") {
      REQUIRE(makeAdapter([] { return false; })().result.type ==
              CaseResult::Type::Failure);
    }
  }

  SECTION("returns success for empty strings") {
    REQUIRE(makeAdapter([] { return std::string(); })().result.type ==
            CaseResult::Type::Success);
  }

  prop("returns failure with the string as message for non-empty strings",
       [] {
         // TODO non-empty generator
         const auto msg = *gen::nonEmpty<std::string>();
         const auto result = makeAdapter([&] { return msg; })();
         RC_ASSERT(result.result.type == CaseResult::Type::Failure);
         RC_ASSERT(descriptionContains(result, msg));
       });

  prop("if a CaseResult is thrown, returns that case result",
       [](const CaseResult &result) {
         RC_ASSERT(makeAdapter([&] { throw result; })().result == result);
       });

  prop("returns a discard result if a GenerationFailure is thrown",
       [](const std::string &msg) {
         const auto result =
             makeAdapter([&] { throw GenerationFailure(msg); })();
         RC_ASSERT(result.result.type == CaseResult::Type::Discard);
         RC_ASSERT(descriptionContains(result, msg));
       });

  prop(
      "returns a failure result with what message if an std::exception is"
      " thrown",
      [](const std::string &msg) {
        const auto result =
            makeAdapter([&] { throw std::runtime_error(msg); })();
        RC_ASSERT(result.result.type == CaseResult::Type::Failure);
        RC_ASSERT(descriptionContains(result, msg));
      });

  prop(
      "returns a failure result with the string as the message if a string"
      " is thrown",
      [](const std::string &msg) {
        const auto result = makeAdapter([&] { throw msg; })();
        RC_ASSERT(result.result.type == CaseResult::Type::Failure);
        RC_ASSERT(descriptionContains(result, msg));
      });

  SECTION("returns a failure result if other values are thrown") {
    const auto result = makeAdapter([&] { throw 1337; })();
    RC_ASSERT(result.result.type == CaseResult::Type::Failure);
  }

  prop("forwards arguments to callable",
       [](int a, const std::string &b, NonCopyable c) {
         const auto expected = std::to_string(a) + b + std::to_string(c.extra);
         const auto adapter =
             makeAdapter([](int d, const std::string &e, NonCopyable f) {
               return std::to_string(d) + e + std::to_string(f.extra);
             });
         const auto result = adapter(std::move(a), std::move(b), std::move(c));
         RC_ASSERT(result.result.description == expected);
       });

  prop("returns any tags that were added",
       [](const std::vector<std::string> &tags) {
         const auto result = makeAdapter([&] {
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

  prop("counterexample contains arguments as tuple",
       [](const GenParams &params) {
         const auto gen = toProperty([=](Fixed<1>, Fixed<2>, Fixed<3>) {});
         const auto shrinkable = gen(params.random, params.size);
         const auto expected =
             toString(std::make_tuple(Fixed<1>(), Fixed<2>(), Fixed<3>()));

         onAnyPath(shrinkable,
                   [&](const ShrinkableResult &value,
                       const ShrinkableResult &shrink) {
                     RC_ASSERT(value.value().example().front().second ==
                               expected);
                   });
       });

  prop("counterexample contains string versions of picked values",
       [](const GenParams &params) {
         const auto n = *gen::inRange<std::size_t>(0, 10);
         const auto gen = toProperty([=] {
           for (std::size_t i = 0; i < n; i++) {
             *gen::arbitrary<Fixed<1337>>();
           }
         });
         const auto shrinkable = gen(params.random, params.size);
         const auto expected = toString(Fixed<1337>());

         onAnyPath(shrinkable,
                   [&](const ShrinkableResult &value,
                       const ShrinkableResult &shrink) {
                     for (const auto &desc : value.value().example()) {
                       RC_ASSERT(desc.second == expected);
                     }
                   });
       });

  prop("counterexample contains type of value generator has no name",
       [](const GenParams &params) {
         const auto gen = toProperty([] { *gen::arbitrary<int>(); });
         const auto shrinkable = gen(params.random, params.size);
         const auto expected = typeToString<int>();

         onAnyPath(shrinkable,
                   [&](const ShrinkableResult &value,
                       const ShrinkableResult &shrink) {
                     RC_ASSERT(value.value().example().front().first ==
                               expected);
                   });
       });

  prop("counterexample contains name of generator if it has one",
       [](const GenParams &params) {
         const auto name = *gen::nonEmpty<std::string>();
         const auto gen = gen::arbitrary<int>().as(name);
         const auto property = toProperty([=] { *gen; });
         const auto shrinkable = property(params.random, params.size);

         onAnyPath(shrinkable,
                   [&](const ShrinkableResult &value,
                       const ShrinkableResult &shrink) {
                     RC_ASSERT(value.value().example().front().first == name);
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
                // Introduce a dummy variable to prevent double-free error on LLVM 8.0.0.
                auto dummy = Gen<int>([=](const Random &, int) -> Shrinkable<int> {
                  throw GenerationFailure(msg);
                });
                *dummy;
              } catch (...) {
              }
            } else {
              *gen::arbitrary<Fixed<1337>>();
            }
          }
        });
        const auto shrinkable = gen(params.random, params.size);
        const std::pair<std::string, std::string> expected("Generation failed",
                                                           msg);

        onAnyPath(
            shrinkable,
            [&](const ShrinkableResult &value, const ShrinkableResult &shrink) {
              RC_ASSERT(value.value().example()[throwIndex] == expected);
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
                         ((std::stoi(desc.example().back().second) % 2) == 0));
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
               RC_ASSERT(toString(desc.tags) == desc.example().back().second);
             });
       });
}
