#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/shrinkable/Create.h"

namespace rc {
namespace test {

// Generator which returns the passed size
inline Gen<int> genSize() {
    return [](const Random &random, int size) {
        return shrinkable::just(size);
    };
};

struct PassedSize { int value; };

// Generator which returns the passed random.
inline Gen<Random> genRandom() {
    return [](const Random &random, int size) {
        return shrinkable::just(random);
    };
};

struct PassedRandom { Random value; };

// Generates a constant number which shrinks by count down towards zero
inline Gen<int> genCountdown(int value) {
    return [=](const Random &random, int size) {
        return shrinkable::shrinkRecur(value, [](int x) {
            return seq::range(x - 1, -1);
        });
    };
};

template<int N>
struct Countdown { int value; };

template<int N>
inline bool operator==(const Countdown<N> &lhs, const Countdown<N> &rhs) {
    return lhs.value == rhs.value;
}

} // namespace test

template<>
struct NewArbitrary<test::PassedSize>
{
    static Gen<test::PassedSize> arbitrary()
    {
        return [](const Random &random, int size) {
            test::PassedSize sz;
            sz.value = size;
            return shrinkable::just(sz);
        };
    }
};

template<>
struct NewArbitrary<test::PassedRandom>
{
    static Gen<test::PassedRandom> arbitrary()
    {
        return [](const Random &random, int size) {
            test::PassedRandom rnd;
            rnd.value = random;
            return shrinkable::just(rnd);
        };
    }
};

template<int N>
struct NewArbitrary<test::Countdown<N>>
{
    static Gen<test::Countdown<N>> arbitrary()
    {
        return newgen::map([](int x) {
            test::Countdown<N> countdown;
            countdown.value = x;
            return countdown;
        }, test::genCountdown(N));
    }
};

} // namespace rc
