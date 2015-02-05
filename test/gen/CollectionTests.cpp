#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Meta.h"
#include "util/Util.h"
#include "util/Predictable.h"

using namespace rc;

struct VectorTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "uses the given generator for elements",
            [] {
                auto size = *gen::ranged<std::size_t>(0, gen::currentSize());
                auto egen = gen::arbitrary<typename T::value_type>();
                auto elements = *gen::noShrink(gen::vector<T>(size, egen));
                for (const auto &e : elements)
                    RC_ASSERT(isArbitraryPredictable(e));
            });

        templatedProp<T>(
            "generates collections of the given size",
            [] {
                auto size = *gen::ranged<std::size_t>(0, gen::currentSize());
                auto egen = gen::arbitrary<typename T::value_type>();
                auto elements = *gen::noShrink(gen::vector<T>(size, egen));
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
                auto size = *gen::ranged<std::size_t>(0, gen::currentSize());
                auto coll = *gen::noShrink(
                    gen::vector<T>(size, gen::arbitrary<Element>()));
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
                auto elements = *gen::noShrink(
                    gen::collection<T>(
                        gen::arbitrary<typename T::value_type>()));
                for (const auto &e : elements)
                    RC_ASSERT(isArbitraryPredictable(e));
            });

        templatedProp<T>(
            "generates empty collections for 0 size",
            [] {
                auto egen = gen::arbitrary<typename T::value_type>();
                auto coll = *gen::noShrink(
                    gen::resize(0, gen::collection<T>(egen)));
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
                auto coll = *gen::noShrink(gen::collection<T>(egen));
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
             auto coll = *gen::noShrink(gen::collection<ArrayT>(egen));
             for (const auto &e : coll)
                 RC_ASSERT(isArbitraryPredictable(e));
         });
}
