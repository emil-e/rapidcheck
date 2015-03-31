#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/Predictable.h"

using namespace rc;

TEST_CASE("gen::tupleOf") {
    prop("uses the provided generators",
         [] {
             auto tuple = *gen::noShrink(gen::tupleOf(gen::constant(1),
                                                      gen::constant(2),
                                                      gen::constant(3),
                                                      gen::constant(4)));
             RC_ASSERT(tuple == std::make_tuple(1, 2, 3, 4));
         });

    prop("works with non-copyable types",
         [] {
             auto tuple = *gen::noShrink(gen::tupleOf(gen::constant(std::string("foobar")),
                                                      gen::constant(123),
                                                      gen::arbitrary<NonCopyable>(),
                                                      gen::arbitrary<NonCopyable>()));
             RC_ASSERT(std::get<0>(tuple) == "foobar");
             RC_ASSERT(std::get<1>(tuple) == 123);
             RC_ASSERT(isArbitraryPredictable(std::get<2>(tuple)));
             RC_ASSERT(isArbitraryPredictable(std::get<3>(tuple)));
         });

    prop("shrinks one element at a time",
         [] (const std::tuple<int, int, int> &tuple) {
             auto seq = gen::tupleOf(gen::arbitrary<int>(),
                                     gen::arbitrary<int>(),
                                     gen::arbitrary<int>()).shrink(tuple);

             auto expected = seq::concat(
                 seq::map(
                     gen::arbitrary<int>().shrink(std::get<0>(tuple)),
                     [&](int x) {
                         return std::make_tuple(x,
                                                std::get<1>(tuple),
                                                std::get<2>(tuple));
                     }),
                 seq::map(
                     gen::arbitrary<int>().shrink(std::get<1>(tuple)),
                     [&](int x) {
                         return std::make_tuple(std::get<0>(tuple),
                                                x,
                                                std::get<2>(tuple));
                     }),
                 seq::map(
                     gen::arbitrary<int>().shrink(std::get<2>(tuple)),
                     [&](int x) {
                     return std::make_tuple(std::get<0>(tuple),
                                            std::get<1>(tuple),
                                            x);
                     }));

             RC_ASSERT(seq == expected);
         });
}


TEST_CASE("gen::pairOf") {
    prop("uses the provided generators",
         [] {
             auto pair = *gen::noShrink(gen::pairOf(gen::constant(1),
                                                    gen::constant(2)));
             RC_ASSERT(pair == std::make_pair(1, 2));
         });

    prop("works with non-copyable types",
         [] {
             auto pair = *gen::noShrink(gen::pairOf(gen::constant(std::string("foobar")),
                                                    gen::arbitrary<NonCopyable>()));
             RC_ASSERT(pair.first == "foobar");
             RC_ASSERT(isArbitraryPredictable(pair.second));
         });

    prop("shrinks one element at a time",
         [] (const std::pair<int, int> &pair) {
             auto seq = gen::pairOf(gen::arbitrary<int>(),
                                   gen::arbitrary<int>()).shrink(pair);

             auto expected = seq::concat(
                 seq::map(
                     gen::arbitrary<int>().shrink(pair.first),
                     [&](int x) {
                         return std::make_pair(x, pair.second);
                     }),
                 seq::map(
                     gen::arbitrary<int>().shrink(pair.second),
                     [&](int x) {
                         return std::make_pair(pair.first, x);
                     }));

             RC_ASSERT(seq == expected);
         });
}
