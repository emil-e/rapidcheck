#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/newgen/Arbitrary.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrinkable/Operations.h"

#include "util/ArbitraryRandom.h"

namespace rc {
namespace test {

// Generator which returns the passed size
inline Gen<int> genSize()
{
    return [](const Random &random, int size) {
        return shrinkable::just(size);
    };
};

struct PassedSize { int value; };

// Generator which returns the passed random.
inline Gen<Random> genRandom()
{
    return [](const Random &random, int size) {
        return shrinkable::just(random);
    };
};

struct PassedRandom { Random value; };

// Generates a number between 0 and the size (inclusive) that shrinks by
// counting down toward zero.
inline Gen<int> genCountdown()
{
    return [=](const Random &random, int size) {
        int n = Random(random).next() % (size + 1);
        return shrinkable::shrinkRecur(n, [](int x) {
            return seq::range(x - 1, -1);
        });
    };
};

// Generates a constant number which shrinks by count down towards zero
inline Gen<int> genFixedCountdown(int value)
{
    return [=](const Random &random, int size) {
        return shrinkable::shrinkRecur(value, [](int x) {
            return seq::range(x - 1, -1);
        });
    };
};

template<int N>
struct FixedCountdown {
    FixedCountdown() : value(0) {}
    FixedCountdown(int x) : value(x) {}

    int value;
};

template<int N>
inline bool operator==(const FixedCountdown<N> &lhs,
                       const FixedCountdown<N> &rhs)
{ return lhs.value == rhs.value; }

template<int N>
inline std::ostream &operator<<(std::ostream &os,
                                const FixedCountdown<N> &value)
{
    os << value.value;
    return os;
}

struct GenParams
{
    Random random;
    int size = 0;
};

inline bool operator==(const GenParams &lhs, const GenParams &rhs)
{ return (lhs.random == rhs.random) && (lhs.size == rhs.size); }

inline bool operator<(const GenParams &lhs, const GenParams &rhs)
{ return std::tie(lhs.random, lhs.size) < std::tie(rhs.random, rhs.size); }

// Generator which returns the passed generation params.
inline Gen<GenParams> genPassedParams()
{
    return [](const Random &random, int size) {
        GenParams params;
        params.random = random;
        params.size = size;
        return shrinkable::just(params);
    };
}

inline std::ostream &operator<<(std::ostream &os, const GenParams &params)
{
    os << "Random: " << params.random << std::endl;
    os << "Size: " << params.size << std::endl;
    return os;
}

// Tries to find a value which matches the predicate and then shrink that to the
// minimum value. Simplified version of what RapidCheck is all about.
template<typename T, typename Predicate>
T searchGen(const Random &random,
            int size,
            const Gen<T> &gen,
            Predicate predicate)
{
    Random r(random);
    for (int tries = 0; tries < 100; tries++) {
        const auto shrinkable = gen(r.split(), size);
        const auto value = shrinkable.value();
        if (!predicate(value))
            continue;
        return shrinkable::findLocalMin(shrinkable, predicate).first;
    }

    RC_DISCARD("Couldn't satisfy predicate");
}

} // namespace test

template<>
class Arbitrary<test::GenParams> : public gen::Generator<test::GenParams>
{
public:
    test::GenParams generate() const override
    {
        test::GenParams params;
        params.random = *gen::arbitrary<Random>();
        params.size = *gen::ranged<int>(0, 200);
        return params;
    }
};

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
struct NewArbitrary<test::FixedCountdown<N>>
{
    static Gen<test::FixedCountdown<N>> arbitrary()
    {
        return newgen::map(test::genFixedCountdown(N), [](int x) {
            test::FixedCountdown<N> countdown;
            countdown.value = x;
            return countdown;
        });
    }
};

} // namespace rc

namespace std {

template<>
struct hash<rc::test::GenParams>
{
    typedef rc::test::GenParams argument_type;
    typedef std::size_t result_type;

    std::size_t operator()(const rc::test::GenParams &params) const
    {
        return
            std::hash<rc::Random>()(params.random) ^
            (std::hash<int>()(params.size) << 1);
    }
};

} // namespace std
