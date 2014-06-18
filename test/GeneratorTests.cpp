#include <catch.hpp>
#include <rapidcheck.h>

#include "ShrinkTestUtils.h"

using namespace rc;

template<typename Callable>
auto cleanRoom(const Callable &callable) -> decltype(callable())
{
    using namespace detail;

    ImplicitParam<param::Size> size;
    if (!size.hasBinding())
        size.let(0);

    ImplicitParam<param::NoShrink> noShrink;

    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(RandomEngine());
    randomEngine->seed(pick<RandomEngine::Atom>());

    ImplicitParam<param::CurrentNode> currentNode;
    currentNode.let(nullptr);

    return callable();
}

TEST_CASE("gen::suchThat") {
    prop("never generates values not satisfying the predicate",
         [] (int max) {
             cleanRoom([=] {
                 int x = pick(gen::suchThat<int>(
                                  [=](int x) { return x < max; }));
                 RC_ASSERT(x < max);
             });
         });
}

TEST_CASE("gen::ranged") {
    prop("never generates values outside of range", [] {
        int min = pick<int>();
        int max = pick(gen::suchThat<int>(
                           [=](int x) { return x > min; }));
        int x = cleanRoom([=]{ return pick(gen::ranged(min, max)); });
        RC_ASSERT((x >= min) && (x < max));
    });

    prop("sometimes generates negative values if in range", [] {
        int min = pick(gen::negative<int>());
        int max = pick(gen::positive<int>());
        cleanRoom([=] {
            while (true)
                RC_SUCCEED_IF(pick(gen::ranged(min, max)) < 0);
        });

        return false;
    });
}

TEST_CASE("gen::nonZero") {
    prop("never generates zero", [] {
        cleanRoom([] { RC_ASSERT(pick(gen::nonZero<int>()) != 0); });
    });
}

TEST_CASE("gen::positive") {
    prop("never generates non-positive", [] {
        cleanRoom([] { RC_ASSERT(pick(gen::positive<int>()) > 0); });
    });
}

TEST_CASE("gen::negative") {
    prop("never generates non-negative", [] {
        cleanRoom([] { RC_ASSERT(pick(gen::negative<int>()) < 0); });
    });
}

TEST_CASE("gen::nonNegative") {
    prop("never generates negative", [] {
        cleanRoom([] { RC_ASSERT(pick(gen::nonNegative<int>()) >= 0); });
    });
}

TEST_CASE("gen::collection") {
    prop("uses the given generator for elements",
         [] (int x) {
             cleanRoom([=] {
                 auto elements =
                     pick(gen::collection<std::vector<int>>(gen::constant(x)));
                 for (int e : elements)
                     RC_ASSERT(e == x);
             });
         });

    SECTION("generates empty collections for 0 size") {
        cleanRoom([] {
            auto coll = pick(gen::resize(0, gen::collection<std::vector<int>>(
                                             gen::arbitrary<int>())));
            REQUIRE(coll.empty());
        });
    }
}

TEST_CASE("gen::character") {
    prop("never generates null characters", [] {
        cleanRoom([] { RC_ASSERT(pick(gen::character<char>()) != '\0'); });
    });

    SECTION("does not shrink 'a'") {
        REQUIRE(!gen::character<char>().shrink('a')->hasNext());
    }

    prop("tries to shrink every value to 'a')", [] {
        char c = pick(gen::character<char>());
        RC_PRE(c != 'a');
        RC_ASSERT(gen::character<char>().shrink(c)->next() == 'a');
    });
}

TEST_CASE("gen::resize") {
    prop("changes the generation size",
         [] (size_t size) {
             auto generator =
                 gen::resize(size, gen::lambda([] { return gen::currentSize(); }));
             cleanRoom([] { RC_ASSERT(pick(generator) == size); });
         });
}

struct MyInt { int value };

class DummyGen : public Generator<int>
{
public:
    int operator()() const override { return m_value++; }
private:
    int m_value = 0;
}
