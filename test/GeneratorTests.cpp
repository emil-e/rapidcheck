#include <catch.hpp>
#include <rapidcheck-catch.h>

#include <array>

#include "util/Util.h"
#include "util/Meta.h"
#include "util/Predictable.h"

using namespace rc;

TEST_CASE("gen::suchThat") {
    prop("never generates values not satisfying the predicate",
         [] (int max) {
             int x = pick(
                 gen::noShrink(
                     gen::suchThat<int>([=](int x) { return x < max; })));
             RC_ASSERT(x < max);
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
                int x = pick(gen::noShrink(gen::ranged(min, max)));
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
                auto generator = gen::noShrink(gen::ranged(min, max));
                while (true)
                    RC_SUCCEED_IF(pick(generator) < 0);

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
             int value = pick(gen::noShrink(gen::oneOf(gen::constant(a),
                                                       gen::constant(b),
                                                       gen::constant(c))));
             RC_ASSERT((value == a) ||
                       (value == b) ||
                       (value == c));
         });

    prop("all generators are eventually used",
         [] (int a, int b, int c) {
             while (true) {
                 int value = pick(gen::noShrink(gen::oneOf(gen::constant(a),
                                                           gen::constant(b),
                                                           gen::constant(c))));
                 if (value == a)
                     break;
             }

             while (true) {
                 int value = pick(gen::noShrink(gen::oneOf(gen::constant(a),
                                                           gen::constant(b),
                                                           gen::constant(c))));
                 if (value == b)
                     break;
             }

             while (true) {
                 int value = pick(gen::noShrink(gen::oneOf(gen::constant(a),
                                                           gen::constant(b),
                                                           gen::constant(c))));
                 if (value == c)
                     break;
             }
         });
}

struct NonZeroProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates zero", [] {
            RC_ASSERT(pick(gen::noShrink(gen::nonZero<T>())) != 0);
        });
    }
};


struct PositiveProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates non-positive", [] {
            RC_ASSERT(pick(gen::noShrink(gen::positive<T>())) > 0);
        });
    }
};

struct NegativeProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates non-negative", [] {
            RC_ASSERT(pick(gen::noShrink(gen::negative<T>())) < 0);
        });
    }
};

struct NonNegativeProperties
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>("never generates negative", [] {
            RC_ASSERT(pick(gen::noShrink(gen::nonNegative<T>())) >= 0);
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
                auto egen = gen::arbitrary<typename T::value_type>();
                auto elements = pick(gen::noShrink(gen::vector<T>(size, egen)));
                for (const auto &e : elements)
                    RC_ASSERT(isArbitraryPredictable(e));
            });

        templatedProp<T>(
            "generates collections of the given size",
            [] {
                auto size = pick(gen::ranged<std::size_t>(0, gen::currentSize()));
                auto egen = gen::arbitrary<typename T::value_type>();
                auto elements = pick(gen::noShrink(gen::vector<T>(size, egen)));
                auto actualSize = std::distance(begin(elements),
                                                end(elements));
                RC_ASSERT(actualSize == size);
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
                auto coll = pick(
                    gen::noShrink(
                        gen::vector<T>(size, gen::arbitrary<Element>())));
                for (const auto &e : coll)
                    RC_ASSERT(isArbitraryPredictable(e));
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
                auto elements = pick(
                    gen::noShrink(
                        gen::collection<T>(
                            gen::arbitrary<typename T::value_type>())));
                    for (const auto &e : elements)
                        RC_ASSERT(isArbitraryPredictable(e));
            });

        templatedProp<T>(
            "generates empty collections for 0 size",
            [] {
                auto egen = gen::arbitrary<typename T::value_type>();
                auto coll = pick(
                    gen::noShrink(gen::resize(0, gen::collection<T>(egen))));
                RC_ASSERT(coll.empty());
            });
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
                auto egen = gen::arbitrary<typename T::value_type>();
                auto coll = pick(gen::noShrink(gen::collection<T>(egen)));
                for (const auto &e : coll)
                    RC_ASSERT(isArbitraryPredictable(e));
            });
    }
};

TEST_CASE("gen::collection") {
    meta::forEachType<CollectionTests,
                      RC_GENERIC_CONTAINERS(Predictable),
                      std::basic_string<Predictable>>();
    meta::forEachType<NonCopyableCollectionTests,
                      RC_GENERIC_CONTAINERS(NonCopyable),
                      std::array<NonCopyable, 100>>();

    // Can't use CollectionTests since that tests for size which is fixed for
    // std::array
    prop("works with std::array",
         [] {
             typedef std::array<Predictable, 100> ArrayT;
             auto egen = gen::arbitrary<Predictable>();
             auto coll = pick(gen::noShrink(gen::collection<ArrayT>(egen)));
             for (const auto &e : coll)
                 RC_ASSERT(isArbitraryPredictable(e));
         });
}

TEST_CASE("gen::resize") {
    prop("changes the generation size",
         [] {
             auto size = pick(gen::positive<int>());
             auto generator =
                 gen::noShrink(
                     gen::resize(
                         size,
                         gen::lambda([] { return gen::currentSize(); })));
             RC_ASSERT(pick(generator) == size);
         });
}

// Always increasing type for testing generatinon ordering. Note that thi is
// very much a hack but it will catch regressions where arguments are not
// generated in the correct order.
struct IncInt { int value; };

template<>
class Arbitrary<IncInt> : public gen::Generator<IncInt>
{
public:
    IncInt generate() const override
    {
        static int value = 0;
        return IncInt { value++ };
    }
};

void show(IncInt x, std::ostream &os) { os << x.value; }

TEST_CASE("gen::anyInvocation") {
    prop("generates arguments in listing order",
         [] {
             auto tuple = pick(
                 gen::noShrink(
                     gen::anyInvocation(
                         [] (IncInt a, IncInt b, IncInt c) {
                             return std::make_tuple(a.value, b.value, c.value);
                         })));

             RC_ASSERT(std::get<0>(tuple) < std::get<1>(tuple));
             RC_ASSERT(std::get<1>(tuple) < std::get<2>(tuple));
         });

    prop("uses the appropriate Arbitrary instance",
         [] {
             auto tuple = pick(
                 gen::noShrink(
                     gen::anyInvocation(
                         [] (const Predictable &a,
                             Predictable &&b,
                             Predictable c)
                         {
                             return std::make_tuple(a.value, b.value, c.value);
                         })));
             RC_ASSERT(std::get<0>(tuple) == Predictable::predictableValue);
             RC_ASSERT(std::get<1>(tuple) == Predictable::predictableValue);
             RC_ASSERT(std::get<2>(tuple) == Predictable::predictableValue);
         });

    prop("uses the return value as the generated value",
         [] {
             int x = pick(
                 gen::noShrink(
                     gen::anyInvocation([] (int a, int b) { return 12345; })));
             RC_ASSERT(x == 12345);
         });
}

TEST_CASE("gen::noShrink") {
    prop("sets the NoShrink parameter",
         [] {
             detail::ImplicitParam<detail::param::NoShrink> noShrink;
             noShrink.let(pick<bool>());
             bool wasNoShrink = pick(gen::noShrink(gen::lambda([]{
                 return *detail::ImplicitParam<detail::param::NoShrink>();
             })));
             RC_ASSERT(wasNoShrink);
         });

    prop("blocks explicit shrinking",
         [] {
             auto generator = gen::arbitrary<int>();
             auto value = pick(generator);
             RC_ASSERT(!gen::noShrink(generator).shrink(value)->hasNext());
         });
}

TEST_CASE("gen::map") {
    prop("maps a generated values from one type to another",
         [] (int input) {
             std::string str(
                 pick( gen::noShrink(
                         gen::map(gen::constant(input),
                                    [] (int x) {
                                      return std::to_string(x);
                                  }))));
             RC_ASSERT(str == std::to_string(input));
         });
}

TEST_CASE("gen::character") {
    prop("never generates null characters", [] {
        RC_ASSERT(pick(gen::noShrink(gen::character<char>())) != '\0');
    });

    SECTION("does not shrink 'a'") {
        REQUIRE(!gen::character<char>().shrink('a')->hasNext());
    }

    prop("first tries to shrink every value to 'a')", [] {
        char c = pick(gen::character<char>());
        RC_PRE(c != 'a');
        RC_ASSERT(gen::character<char>().shrink(c)->next() == 'a');
    });
}

TEST_CASE("gen::rescue") {
    prop("converts exceptions to generated values",
         [] (int x) {
             auto generator = gen::lambda([=] {
                 throw std::to_string(x);
                 return std::string("");
             });

             std::string str(
                 pick(gen::noShrink(
                          gen::rescue<std::string>(
                              generator,
                              [] (const std::string &ex) {
                                  return ex;
                              }))));

             RC_ASSERT(str == std::to_string(x));
         });
}

TEST_CASE("gen::constant") {
    prop("always returns the constant value",
         [] (int x) {
             auto generator = gen::constant(x);
             for (int i = 0; i < gen::currentSize(); i++)
                 RC_ASSERT(pick(generator) == x);
         });
}

TEST_CASE("gen::lambda") {
    prop("generates the return value of the given callable",
         [] (int x) {
             auto generator = gen::lambda([=] { return x; });
             for (int i = 0; i < gen::currentSize(); i++)
                 RC_ASSERT(pick(generator) == x);
         });
}

TEST_CASE("gen::tupleOf") {
    prop("uses the provided generators",
         [] {
             auto tuple = pick(gen::noShrink(gen::tupleOf(gen::constant(1),
                                                          gen::constant(2),
                                                          gen::constant(3),
                                                          gen::constant(4))));
             RC_ASSERT(tuple == std::make_tuple(1, 2, 3, 4));
         });

    prop("works with non-copyable types",
         [] {
             auto tuple = pick(
                 gen::noShrink(gen::tupleOf(gen::constant(std::string("foobar")),
                                            gen::constant(123),
                                            gen::arbitrary<NonCopyable>(),
                                            gen::arbitrary<NonCopyable>())));
             RC_ASSERT(std::get<0>(tuple) == "foobar");
             RC_ASSERT(std::get<1>(tuple) == 123);
             RC_ASSERT(isArbitraryPredictable(std::get<2>(tuple)));
             RC_ASSERT(isArbitraryPredictable(std::get<3>(tuple)));
         });

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
             auto pair = pick(gen::noShrink(gen::pairOf(gen::constant(1),
                                                        gen::constant(2))));
             RC_ASSERT(pair == std::make_pair(1, 2));
         });

    prop("works with non-copyable types",
         [] {
            auto pair = pick(gen::noShrink(gen::pairOf(gen::constant(std::string("foobar")),
                                                       gen::arbitrary<NonCopyable>())));
            RC_ASSERT(pair.first == "foobar");
            RC_ASSERT(isArbitraryPredictable(pair.second));
         });

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
