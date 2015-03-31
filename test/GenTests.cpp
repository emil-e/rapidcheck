#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/Gen.h"
#include "rapidcheck/shrinkable/Create.h"

#include "util/ArbitraryRandom.h"

using namespace rc;
using namespace rc::detail;
using namespace rc::newgen::detail;

struct MockGenerationHandler : public GenerationHandler
{
    Any onGenerate(const Gen<Any> &gen) override
    {
        wasCalled = true;
        passedGenerator = gen;
        return returnValue;
    }

    bool wasCalled = false;
    Gen<Any> passedGenerator = Gen<Any>(
        fn::constant(shrinkable::just(Any::of(0))));
    Any returnValue;
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
    }

    SECTION("operator*") {
        ImplicitScope scope;

        SECTION("by default throws exception") {
            Gen<int> gen(fn::constant(shrinkable::just(0)));
            REQUIRE_THROWS(*gen);
        }

        MockGenerationHandler handler;
        handler.returnValue = Any::of(456);
        ImplicitParam<rc::newgen::detail::param::CurrentHandler> letHandler(
            &handler);
        Gen<int> gen(fn::constant(shrinkable::just(1337)));
        int x = *gen;

        SECTION("passes erased self to onGenerate") {
            auto result = shrinkable::map(
                handler.passedGenerator(Random(), 0),
                [](Any &&any) { return std::move(any.get<int>()); });
            REQUIRE(result == shrinkable::just(1337));
            REQUIRE(handler.wasCalled);
        }

        SECTION("returns what is returned by onGenerate") {
            RC_ASSERT(x == 456);
        }
    }
}
