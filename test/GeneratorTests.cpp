#include <catch.hpp>
#include <rapidcheck.h>

#include "util/Util.h"
#include "util/Meta.h"
#include "util/Predictable.h"

using namespace rc;

TEST_CASE("gen::suchThat") {
    prop("never generates values not satisfying the predicate",
         [] (int max) {
             testEnv([=] {
                 int x = pick(gen::suchThat<int>(
                                  [=](int x) { return x < max; }));
                 RC_ASSERT(x < max);
             });
         });
}

struct RangedProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "never generates values outside of range", [] {
                int min = pick<int>();
                int max = pick(gen::suchThat<int>(
                                   [=](int x) { return x > min; }));
                int x = testEnv([=]{ return pick(gen::ranged(min, max)); });
                RC_ASSERT((x >= min) && (x < max));
            });
    }
};

struct SignedRangedProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "sometimes generates negative values if in range", [] {
                int min = pick(gen::negative<int>());
                int max = pick(gen::positive<int>());
                testEnv([=] {
                    while (true)
                        RC_SUCCEED_IF(pick(gen::ranged(min, max)) < 0);
                });

                return false;
            });
    }
};

TEST_CASE("gen::ranged") {
    meta::forEachType<RangedProperties, RC_NUMERIC_TYPES>();
    meta::forEachType<SignedRangedProperties, RC_SIGNED_TYPES>();
}

TEST_CASE("gen::oneOf") {
    prop("only uses the given generators",
         [] (int a, int b, int c) {
             testEnv([=] {
                 int value = pick(gen::oneOf(gen::constant(a),
                                             gen::constant(b),
                                             gen::constant(c)));
                 RC_ASSERT((value == a) ||
                           (value == b) ||
                           (value == c));
             });
         });

    prop("all generators are eventually used",
         [] (int a, int b, int c) {
             testEnv([=] {
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

struct NonZeroProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates zero", [] {
            testEnv([] { RC_ASSERT(pick(gen::nonZero<T>()) != 0); });
        });
    }
};


struct PositiveProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates non-positive", [] {
            testEnv([] { RC_ASSERT(pick(gen::positive<T>()) > 0); });
        });
    }
};

struct NegativeProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates non-negative", [] {
            testEnv([] { RC_ASSERT(pick(gen::negative<T>()) < 0); });
        });
    }
};

struct NonNegativeProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates negative", [] {
            testEnv([] { RC_ASSERT(pick(gen::nonNegative<T>()) >= 0); });
        });
    }
};

TEST_CASE("gen::nonZero") {
    meta::forEachType<NonZeroProperties, RC_NUMERIC_TYPES>();
}

TEST_CASE("gen::positive") {
    meta::forEachType<PositiveProperties, RC_NUMERIC_TYPES>();
}

TEST_CASE("gen::negative") {
    meta::forEachType<NegativeProperties, RC_SIGNED_TYPES>();
}

TEST_CASE("gen::nonNegative") {
    meta::forEachType<NonNegativeProperties, RC_NUMERIC_TYPES>();
}

struct VectorTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "uses the given generator for elements",
            [] {
                auto size = pick(gen::ranged<std::size_t>(0, gen::currentSize()));
                testEnv([=] {
                    auto egen = gen::arbitrary<typename T::value_type>();
                    auto elements = pick(gen::vector<T>(size, egen));
                    for (const auto &e : elements)
                        RC_ASSERT(isArbitraryPredictable(e));
                });
            });

        templatedProp<T>(
            "generates collections of the given size",
            [] {
                auto size = pick(gen::ranged<std::size_t>(0, gen::currentSize()));
                testEnv([=] {
                    auto egen = gen::arbitrary<typename T::value_type>();
                    auto elements = pick(gen::vector<T>(size, egen));
                    auto actualSize = std::distance(begin(elements),
                                                    end(elements));
                    RC_ASSERT(actualSize == size);
                });
            });
    }
};

struct NonCopyableVectorTests
{
    template<typename T>
    static void exec()
    {
        typedef typename T::value_type Element;

        templatedProp<T>(
            "works with non-copyable types",
            [] {
                auto size = pick(gen::ranged<std::size_t>(0, gen::currentSize()));
                testEnv([=] {
                    auto coll = pick(gen::vector<T>(size, gen::arbitrary<Element>()));
                    for (const auto &e : coll)
                        RC_ASSERT(isArbitraryPredictable(e));
                });
            });
    }
};

TEST_CASE("gen::vector") {
    meta::forEachType<VectorTests,
                      RC_GENERIC_CONTAINERS(Predictable),
                      std::basic_string<Predictable>>();
    meta::forEachType<NonCopyableVectorTests,
                      RC_GENERIC_CONTAINERS(NonCopyable)>();
}

struct CollectionTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "uses the given generator for elements",
            [] {
                testEnv([=] {
                    auto elements = pick(
                        gen::collection<T>(
                            gen::arbitrary<typename T::value_type>()));
                    for (const auto &e : elements)
                        RC_ASSERT(isArbitraryPredictable(e));
                });
            });

        TEMPLATED_SECTION(T, "generates empty collections for 0 size") {
            testEnv([] {
                auto egen =
                    gen::arbitrary<typename T::value_type>();
                auto coll = pick(gen::resize(0, gen::collection<T>(egen)));
                REQUIRE(coll.empty());
            });
        }
    }
};

struct NonCopyableCollectionTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "works with non-copyable types",
            [] {
                testEnv([] {
                    auto egen = gen::arbitrary<typename T::value_type>();
                    auto coll = pick(gen::collection<T>(egen));
                    for (const auto &e : coll)
                        RC_ASSERT(isArbitraryPredictable(e));
                });
            });
    }
};

TEST_CASE("gen::collection") {
    meta::forEachType<CollectionTests,
                      RC_GENERIC_CONTAINERS(Predictable),
                      std::basic_string<Predictable>>();
    meta::forEachType<NonCopyableCollectionTests,
                      RC_GENERIC_CONTAINERS(NonCopyable)>();
}

TEST_CASE("gen::resize") {
    prop("changes the generation size",
         [] {
             auto size = pick(gen::positive<int>());
             auto generator =
                 gen::resize(size, gen::lambda([] { return gen::currentSize(); }));
             testEnv([&] { RC_ASSERT(pick(generator) == size); });
         });
}

// Hackish always increasing type for testing generatinon ordering
struct MyIncInt { int value; };

template<>
class Arbitrary<MyIncInt> : public gen::Generator<MyIncInt>
{
public:
    MyIncInt generate() const override
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
    MyConstInt generate() const override
    { return MyConstInt { 1337 }; }
};

void show(MyConstInt x, std::ostream &os) { os << x.value; }


TEST_CASE("gen::anyInvocation") {
    testEnv([] {
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
    testEnv([] {
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
             testEnv([=] {
                 std::string str(pick(
                     gen::map(gen::constant(input),
                              [] (int x) { return std::to_string(x); })));
                 RC_ASSERT(str == std::to_string(input));
             });
         });
}

TEST_CASE("gen::character") {
    prop("never generates null characters", [] {
        testEnv([] { RC_ASSERT(pick(gen::character<char>()) != '\0'); });
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
             testEnv([=] {
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

TEST_CASE("gen::constant") {
    prop("always returns the constant value",
         [] (int x) {
             testEnv([=] {
                 auto generator = gen::constant(x);
                 for (int i = 0; i < gen::currentSize(); i++)
                     RC_ASSERT(pick(generator) == x);
             });
         });
}

TEST_CASE("gen::lambda") {
    prop("generates the return value of the given callable",
         [] (int x) {
             testEnv([=] {
                 auto generator = gen::lambda([=] { return x; });
                 for (int i = 0; i < gen::currentSize(); i++)
                     RC_ASSERT(pick(generator) == x);
             });
         });
}

TEST_CASE("gen::tupleOf") {
    prop("uses the provided generators",
         [] {
             testEnv([] {
                 auto tuple = pick(gen::tupleOf(gen::constant(1),
                                                gen::constant(2),
                                                gen::constant(3),
                                                gen::constant(4)));
                 RC_ASSERT(tuple == std::make_tuple(1, 2, 3, 4));
             });
         });

    SECTION("works with non-copyable types") {
        testEnv([] {
            auto tuple = pick(
                gen::tupleOf(gen::constant(std::string("foobar")),
                             gen::constant(123),
                             gen::arbitrary<NonCopyable>(),
                             gen::arbitrary<NonCopyable>()));
            RC_ASSERT(std::get<0>(tuple) == "foobar");
            RC_ASSERT(std::get<1>(tuple) == 123);
            RC_ASSERT(isArbitraryPredictable(std::get<2>(tuple)));
            RC_ASSERT(isArbitraryPredictable(std::get<3>(tuple)));
        });
    }

    prop("shrinks one element at a time",
         [] (const std::tuple<int, int, int> &tuple) {
             auto it = gen::tupleOf(gen::arbitrary<int>(),
                                    gen::arbitrary<int>(),
                                    gen::arbitrary<int>()).shrink(tuple);

             auto eit = gen::arbitrary<int>().shrink(std::get<0>(tuple));
             while (eit->hasNext()) {
                 RC_ASSERT(it->hasNext());
                 auto expected = std::make_tuple(eit->next(),
                                                 std::get<1>(tuple),
                                                 std::get<2>(tuple));
                 RC_ASSERT(it->next() == expected);
             }

             eit = gen::arbitrary<int>().shrink(std::get<1>(tuple));
             while (eit->hasNext()) {
                 RC_ASSERT(it->hasNext());
                 auto expected = std::make_tuple(std::get<0>(tuple),
                                                 eit->next(),
                                                 std::get<2>(tuple));
                 RC_ASSERT(it->next() == expected);
             }

             eit = gen::arbitrary<int>().shrink(std::get<2>(tuple));
             while (eit->hasNext()) {
                 RC_ASSERT(it->hasNext());
                 auto expected = std::make_tuple(std::get<0>(tuple),
                                                 std::get<1>(tuple),
                                                 eit->next());
                 RC_ASSERT(it->next() == expected);
             }

             RC_ASSERT(!it->hasNext());
         });
}


TEST_CASE("gen::pairOf") {
    prop("uses the provided generators",
         [] {
             testEnv([] {
                 auto pair = pick(gen::pairOf(gen::constant(1),
                                              gen::constant(2)));
                 RC_ASSERT(pair == std::make_pair(1, 2));
             });
         });

    SECTION("works with non-copyable types") {
        testEnv([] {
            auto pair = pick(gen::pairOf(gen::constant(std::string("foobar")),
                                         gen::arbitrary<NonCopyable>()));
            RC_ASSERT(pair.first == "foobar");
            RC_ASSERT(isArbitraryPredictable(pair.second));
        });
    }

    prop("shrinks one element at a time",
         [] (const std::pair<int, int> &pair) {
             auto it = gen::pairOf(gen::arbitrary<int>(),
                                    gen::arbitrary<int>()).shrink(pair);

             auto eit = gen::arbitrary<int>().shrink(pair.first);
             while (eit->hasNext()) {
                 RC_ASSERT(it->hasNext());
                 auto expected = std::make_pair(eit->next(), pair.second);
                 RC_ASSERT(it->next() == expected);
             }

             eit = gen::arbitrary<int>().shrink(pair.second);
             while (eit->hasNext()) {
                 RC_ASSERT(it->hasNext());
                 auto expected = std::make_pair(pair.first, eit->next());
                 RC_ASSERT(it->next() == expected);
             }

             RC_ASSERT(!it->hasNext());
         });
}
