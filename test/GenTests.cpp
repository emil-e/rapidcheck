#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/Gen.h"
#include "rapidcheck/shrinkable/Create.h"

#include "util/ArbitraryRandom.h"
#include "util/DestructNotifier.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;
using namespace rc::gen::detail;

namespace {

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

Gen<int> makeDummyGen() { return fn::constant(shrinkable::just(0)); }

} // namespace

TEST_CASE("Gen") {
  SECTION("impl constructor") {
    SECTION("constructs with empty name") {
      const auto gen = makeDummyGen();
      REQUIRE(gen.name().empty());
    }
  }

  SECTION("operator()") {
    prop("passes the arguments to the functor",
         [](const Random &random, int size) {
           bool called = false;
           Random passedRandom;
           int passedSize;
           Gen<int> gen([&](const Random &lambdaRandom, int lambdaSize) {
             called = true;
             passedRandom = lambdaRandom;
             passedSize = lambdaSize;
             return shrinkable::just(0);
           });

           gen(random, size);
           RC_ASSERT(called);
           RC_ASSERT(passedRandom == random);
           RC_ASSERT(passedSize == size);
         });

    prop("returns the value returned by the functor",
         [](const Random &random, int size, int x) {
           Gen<int> gen([=](const Random &lambdaRandom, int lambdaSize) {
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

    SECTION("passed erased self has same name") {
      REQUIRE(handler.passedGenerator.name() == gen.name());
    }

    SECTION("returns what is returned by onGenerate") { RC_ASSERT(x == 456); }
  }

  SECTION("as") {
    SECTION("has implementation identical to original generator") {
      Gen<std::string> g1(
          fn::constant(shrinkable::just(std::string("foobar"))));
      const auto g2 = g1.as("some other name");
      REQUIRE(g2(Random(), 0) == shrinkable::just(std::string("foobar")));
    }

    SECTION("returns generator with given name") {
      REQUIRE(makeDummyGen().as("foobar").name() == "foobar");
    }
  }

  SECTION("copy constructor") {
    SECTION("retains other impl") {
      std::vector<std::string> log;
      Maybe<Gen<DestructNotifier>> g1 =
          gen::just(DestructNotifier("foobar", &log));
      REQUIRE(log.empty());

      const auto g2 = g1;
      REQUIRE(log.empty());

      g1.reset();
      REQUIRE(log.empty());
    }

    SECTION("copies name") {
      const auto g1 = makeDummyGen().as("foobar");
      const auto g2 = g1;
      REQUIRE(g2.name() == "foobar");
    }
  }

  SECTION("copy assignment operator") {
    SECTION("derefs owned impl and retains rhs impl") {
      std::vector<std::string> log;
      Maybe<Gen<DestructNotifier>> g1 =
          gen::just(DestructNotifier("1", &log));
      Maybe<Gen<DestructNotifier>> g2 =
        gen::just(DestructNotifier("2", &log));
      REQUIRE(log.empty());

      g2 = g1;
      REQUIRE(log == std::vector<std::string>{"2"});

      g1.reset();
      REQUIRE(log == std::vector<std::string>{"2"});
    }

    SECTION("copies name") {
      const auto g1 = makeDummyGen().as("foobar");
      auto g2 = makeDummyGen();
      g2 = g1;
      REQUIRE(g2.name() == "foobar");
    }

    SECTION("self assignment leaves value unchanged") {
      const auto shrinkable = shrinkable::just(1337);
      Gen<int> gen([=](const Random &random, int size) { return shrinkable; });
      auto &ref = gen;
      gen = ref;
      REQUIRE(gen(Random(), 0) == shrinkable);
    }
  }

  SECTION("move constructor") {
    SECTION("steals other impl") {
      std::vector<std::string> log;
      auto g1 = gen::just(DestructNotifier("foobar", &log));

      {
        const auto g2 = std::move(g1);
        REQUIRE(log.empty());
      }

      REQUIRE(log == std::vector<std::string>{"foobar"});
    }

    SECTION("has same name as moved from generator") {
      const auto g1 = makeDummyGen().as("foobar");
      const auto g2 = std::move(g1);
      REQUIRE(g2.name() == "foobar");
    }
  }

  SECTION("move assignment operator") {
    SECTION("derefs owned impl and steals rhs impl") {
      std::vector<std::string> log;
      auto g1 = gen::just(DestructNotifier("1", &log));

      {
        auto g2 = gen::just(DestructNotifier("2", &log));
        g2 = std::move(g1);
        REQUIRE(log == std::vector<std::string>{"2"});
      }

      REQUIRE(log == (std::vector<std::string>{"2", "1"}));
    }

    SECTION("has same name as moved from generator") {
      const auto g1 = makeDummyGen().as("foobar");
      auto g2 = makeDummyGen();
      g2 = std::move(g1);
      REQUIRE(g2.name() == "foobar");
    }
  }

  SECTION("destructor") {
    std::vector<std::string> log;

    { const auto g1 = gen::just(DestructNotifier("foobar", &log)); }

    REQUIRE(log == std::vector<std::string>{"foobar"});
  }
}
