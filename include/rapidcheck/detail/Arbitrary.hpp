#pragma once

#include <limits>
#include <type_traits>
#include <map>

#include "RandomEngine.h"

namespace rc {

template<typename T>
class Arbitrary : public gen::Generator<T>
{
public:
    static_assert(std::is_integral<T>::value,
                  "No specialization of Arbitrary for type");

    T operator()() const override
    {
        size_t size = std::min(gen::currentSize(), gen::kReferenceSize);
        detail::RandomEngine::Atom r;
        // TODO this switching shouldn't be done here.
        if (detail::RoseNode::hasCurrent()) {
            r = detail::RoseNode::current().atom();
        } else {
            detail::ImplicitParam<detail::param::RandomEngine> randomEngine;
            r = randomEngine->nextAtom();
        }

        // We vary the size by using different number of bits. This way, we can be
        // that the max value can also be generated.
        int nBits = (size * std::numeric_limits<T>::digits) / gen::kReferenceSize;
        if (nBits == 0)
            return 0;
        constexpr detail::RandomEngine::Atom randIntMax =
            std::numeric_limits<detail::RandomEngine::Atom>::max();
        detail::RandomEngine::Atom mask = ~((randIntMax - 1) << (nBits - 1));

        T x = static_cast<T>(r & mask);
        if (std::numeric_limits<T>::is_signed)
        {
            // Use the topmost bit as the signed bit. Even in the case of a signed
            // 64-bit integer, it won't be used since it actually IS the sign bit.
            constexpr int basicBits =
                std::numeric_limits<detail::RandomEngine::Atom>::digits;
            x *= ((r >> (basicBits - 1)) == 0) ? 1 : -1;
        }

        return x;
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        return shrink::unfold(
            value,
            [=](T i) { return i != 0; },
            [=](T i) { return std::make_pair(static_cast<T>(value - i), i / 2); });
    }
};

// Base for float and double arbitrary instances
template<typename T>
class ArbitraryReal : public gen::Generator<T>
{
public:
    T operator()() const override
    {
        int64_t i = pick(gen::arbitrary<int64_t>());
        T x = static_cast<T>(i) / std::numeric_limits<int64_t>::max();
        return std::pow<T>(2.0, gen::currentSize()) * x;
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        std::vector<T> constants;

        T truncated = std::trunc(value);
        if (std::abs(truncated) < std::abs(value))
            constants.push_back(truncated);

        if (value < 0)
            constants.push_back(-value);

        return shrink::constant(constants);
    }
};

template<>
class Arbitrary<float> : public ArbitraryReal<float> {};

template<>
class Arbitrary<double> : public ArbitraryReal<double> {};

template<>
class Arbitrary<bool> : public gen::Generator<bool>
{
public:
    bool operator()() const override
    { return (pick(resize(gen::kReferenceSize, gen::arbitrary<uint8_t>())) & 0x1) == 0; }
};

template<typename T1, typename T2>
class Arbitrary<std::pair<T1, T2>> : public gen::Generator<std::pair<T1, T2>>
{
public:
    std::pair<T1, T2> operator()() const override
    { return std::make_pair(pick<T1>(), pick<T2>()); }

    shrink::IteratorUP<std::pair<T1, T2>>
    shrink(std::pair<T1, T2> pair) const override
    {
        return shrink::sequentially(
            shrink::map(gen::arbitrary<T1>().shrink(pair.first),
                      [=](T1 x) { return std::make_pair(x, pair.second); }),
            shrink::map(gen::arbitrary<T2>().shrink(pair.second),
                      [=](T2 x) { return std::make_pair(pair.first, x); }));
    }
};

template<typename Coll, typename ValueType>
class ArbitraryCollection : public gen::Generator<Coll>
{
public:
    Coll operator()() const override
    { return pick(gen::collection<Coll>(gen::arbitrary<ValueType>())); }

    shrink::IteratorUP<Coll> shrink(Coll value) const override
    { return gen::collection<Coll>(gen::arbitrary<ValueType>()).shrink(value); }
};

// std::vector
template<typename T, typename Alloc>
class Arbitrary<std::vector<T, Alloc>>
    : public ArbitraryCollection<std::vector<T, Alloc>, T> {};

// std::map
template<typename Key, typename T, typename Compare, typename Alloc>
class Arbitrary<std::map<Key, T, Compare, Alloc>>
    : public ArbitraryCollection<std::map<Key, T, Compare, Alloc>,
                                 std::pair<Key, T>> {};

// std::basic_string
template<typename T, typename Traits, typename Alloc>
class Arbitrary<std::basic_string<T, Traits, Alloc>>
    : public gen::Generator<std::basic_string<T, Traits, Alloc>>
{
public:
    typedef std::basic_string<T, Traits, Alloc> StringType;

    StringType operator()() const override
    { return pick(gen::collection<StringType>(gen::character<T>())); }

    shrink::IteratorUP<StringType> shrink(StringType value) const override
    { return gen::collection<StringType>(gen::character<T>()).shrink(value); }
};

} // namespace rc
