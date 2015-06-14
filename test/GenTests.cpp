#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/Gen.h"
#include "rapidcheck/shrinkable/Create.h"

#include "util/ArbitraryRandom.h"
#include "util/DestructNotifier.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;
using namespace rc::gen::detail;

struct MockGenerationHandler : public GenerationHandler {
  Any onGenerate(const Gen<Any> &gen) override {
    wasCalled = true;
    passedGenerator = gen;
    return Any::of(returnValue);
  }

  bool wasCalled = false;
  Gen<Any> passedGenerator = Gen<Any>([](const Random &, int) {
    return shrinkable::lambda([] { return Any::of(0); });
  });
  int returnValue;
};

TEST_CASE("Gen") {
  SECTION("operator()") {
    prop("passes the arguments to the functor",
         [](const Random &random, int size) {
           bool called = false;
           Random passedRandom;
           int passedSize;
           Gen<int> gen([&](const Random &random, int size) {
             called = true;
             passedRandom = random;
             passedSize = size;
             return shrinkable::just(0);
           });

           gen(random, size);
           RC_ASSERT(called);
           RC_ASSERT(passedRandom == random);
           RC_ASSERT(passedSize == size);
         });

    prop("returns the value returned by the functor",
         [](const Random &random, int size, int x) {
           Gen<int> gen([=](const Random &random, int size) {
             return shrinkable::just(x);
           });

           RC_ASSERT(gen(random, size) == shrinkable::just(x));
         });

    prop(
        "if exception is thrown in generation function, shrinkable is"
        " returned that rethrows the exception on call to value()",
        [](const std::string &message) {
          Gen<int> gen([=](const Random &random, int size) -> Shrinkable<int> {
            throw GenerationFailure(message);
          });

          const auto shrinkable = gen(Random(), 0);
          try {
            shrinkable.value();
          } catch (const GenerationFailure &e) {
            RC_ASSERT(e.what() == message);
            RC_SUCCEED("Threw correct exception");
          }
          RC_FAIL("Did not throw correct exception");
        });
  }

  SECTION("operator*") {
    ImplicitScope scope;

    SECTION("by default throws exception") {
      Gen<int> gen(fn::constant(shrinkable::just(0)));
      REQUIRE_THROWS(*gen);
    }

    MockGenerationHandler handler;
    handler.returnValue = 456;
    ImplicitParam<rc::gen::detail::param::CurrentHandler> letHandler(&handler);
    Gen<int> gen(fn::constant(shrinkable::just(1337)));
    int x = *gen;

    SECTION("passes erased self to onGenerate") {
      auto result =
          shrinkable::map(handler.passedGenerator(Random(), 0),
                          [](Any &&any) { return std::move(any.get<int>()); });
      REQUIRE(result == shrinkable::just(1337));
      REQUIRE(handler.wasCalled);
    }

    SECTION("returns what is returned by onGenerate") { RC_ASSERT(x == 456); }
  }

  SECTION("retains implementation object until no copies remain") {
    std::vector<std::string> log;
    Maybe<Gen<DestructNotifier>> s1 =
        gen::just(DestructNotifier("foobar", &log));
    REQUIRE(log.empty());

    Maybe<Gen<DestructNotifier>> s2 = s1;
    REQUIRE(log.empty());

    s1.reset();
    REQUIRE(log.empty());

    s2.reset();
    REQUIRE(log.size() == 1);
    REQUIRE(log[0] == "foobar");
  }

  SECTION("moving steals reference") {
    std::vector<std::string> log;
    auto s1 = gen::just(DestructNotifier("foobar", &log));

    {
      const auto s2 = std::move(s1);
      REQUIRE(log.empty());
    }

    REQUIRE(log.size() == 1);
    REQUIRE(log[0] == "foobar");
  }

  SECTION("self assignment leaves value unchanged") {
    const auto shrinkable = shrinkable::just(1337);
    Gen<int> gen([=](const Random &random, int size) { return shrinkable; });
    gen = gen;
    REQUIRE(gen(Random(), 0) == shrinkable);
  }
}
