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

TEST_CASE("gen::oneOf") {
    prop("only uses the given generators",
         [] (int a, int b, int c) {
             cleanRoom([=] {
                 int value = pick(gen::oneOf(gen::constant(a),
                                             gen::constant(b),
                                             gen::constant(c)));
                 RC_ASSERT((value == a) ||
                           (value == b) ||
                           (value == c));
             });
         });

    prop("all values are eventually generated",
         [] (int a, int b, int c) {
             cleanRoom([=] {
                 while (true) {
                     int value = pick(gen::oneOf(gen::constant(a),
                                                 gen::constant(b),
                                                 gen::constant(c)));
                     if (value == a)
                         break;
                 }

                 while (true) {
                     int value = pick(gen::oneOf(gen::constant(a),
                                                 gen::constant(b),
                                                 gen::constant(c)));
                     if (value == b)
                         break;
                 }

                 while (true) {
                     int value = pick(gen::oneOf(gen::constant(a),
                                                 gen::constant(b),
                                                 gen::constant(c)));
                     if (value == c)
                         break;
                 }
             });
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

TEST_CASE("gen::resize") {
    prop("changes the generation size",
         [] (size_t size) {
             auto generator =
                 gen::resize(size, gen::lambda([] { return gen::currentSize(); }));
             cleanRoom([&] { RC_ASSERT(pick(generator) == size); });
         });
}

// Hackish always increasing type for testing generatinon ordering
struct MyIncInt { int value; };

template<>
class Arbitrary<MyIncInt> : public gen::Generator<MyIncInt>
{
public:
    MyIncInt operator()() const override
    { return MyIncInt { m_value++ }; }

private:
    static int m_value;
};

int Arbitrary<MyIncInt>::m_value = 0;

void show(MyIncInt x, std::ostream &os) { os << x.value; }

// Always-same for testing correct generator usage
struct MyConstInt { int value; };

template<>
class Arbitrary<MyConstInt> : public gen::Generator<MyConstInt>
{
public:
    MyConstInt operator()() const override
    { return MyConstInt { 1337 }; }
};

void show(MyConstInt x, std::ostream &os) { os << x.value; }


TEST_CASE("gen::anyInvocation") {
    cleanRoom([] {
        SECTION("generates arguments in listing order") {
            pick(gen::anyInvocation(
                     [] (MyIncInt a, MyIncInt b, MyIncInt c) {
                         REQUIRE(a.value < b.value);
                         REQUIRE(b.value < c.value);
                         return 0;
                     }));
        }

        SECTION("uses the appropriate Arbitrary instance") {
            pick(gen::anyInvocation(
                     [] (const MyConstInt &a, MyConstInt &&b, MyConstInt c) {
                         REQUIRE(a.value == 1337);
                         REQUIRE(b.value == 1337);
                         REQUIRE(c.value == 1337);
                         return 0;
                     }));
        }

        SECTION("uses the return value as the generated value") {
            int x = pick(gen::anyInvocation([] (int a, int b) { return 12345; }));
            REQUIRE(x == 12345);
        }
    });
}

TEST_CASE("gen::noShrink") {
    cleanRoom([] {
        SECTION("sets the NoShrink parameter") {
            detail::ImplicitParam<detail::param::NoShrink> noShrink;
            noShrink.let(false);
            pick(gen::noShrink(gen::lambda([&]{
                REQUIRE(*noShrink == true);
                return 0;
            })));
        }
    });
}

TEST_CASE("gen::map") {
    prop("maps a generated values from one type to another",
         [] (int input) {
             cleanRoom([=] {
                 std::string str(pick(
                     gen::map(gen::constant(input),
                              [] (int x) { return std::to_string(x); })));
                 RC_ASSERT(str == std::to_string(input));
             });
         });
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

TEST_CASE("gen::rescue") {
    prop("converts exceptions to generated values",
         [] (int x) {
             cleanRoom([=] {
                 auto generator = gen::lambda([=] {
                     throw std::to_string(x);
                     return std::string("");
                 });

                 std::string str(
                     pick(gen::rescue<std::string>(
                              generator,
                              [] (const std::string &ex) {
                                  return ex;
                              })));

                 RC_ASSERT(str == std::to_string(x));
             });
         });
}
