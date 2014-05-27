#pragma once

#include <limits>
#include <type_traits>

#include "rapidcheck/Check.h"

namespace rc {

template<typename T> class Arbitrary;

namespace detail {

// Signed integer generation
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type defaultGenerate()
{
    size_t size = std::min(currentSize(), kReferenceSize);
    RandomEngine::Atom r;
    // TODO this switching shouldn't be done here.
    if (RoseNode::hasCurrent()) {
        r = RoseNode::current().atom();
    } else {
        ImplicitParam<param::RandomEngine> randomEngine;
        r = randomEngine->nextAtom();
    }

    // We vary the size by using different number of bits. This way, we can be
    // that the max value can also be generated.
    int nBits = (size * std::numeric_limits<T>::digits) / kReferenceSize;
    if (nBits == 0)
        return 0;
    constexpr RandomEngine::Atom randIntMax =
        std::numeric_limits<RandomEngine::Atom>::max();
    RandomEngine::Atom mask = ~((randIntMax - 1) << (nBits - 1));

    T x = static_cast<T>(r & mask);
    if (std::numeric_limits<T>::is_signed)
    {
        // Use the topmost bit as the signed bit. Even in the case of a signed
        // 64-bit integer, it won't be used since it actually IS the sign bit.
        constexpr int basicBits =
            std::numeric_limits<RandomEngine::Atom>::digits;
        x *= ((r >> (basicBits - 1)) == 0) ? 1 : -1;
    }

    return x;
}

// Default catch all
template<typename T>
typename std::enable_if<
    !std::is_integral<T>::value,
    ShrinkIteratorUP<T>>::type
defaultShrink(const T &value)
{
    return ShrinkIteratorUP<T>(new NullIterator<T>());
}

template<typename T>
typename std::enable_if<
    std::is_integral<T>::value,
    ShrinkIteratorUP<T>>::type
defaultShrink(const T &value)
{
    return unfold(
        value,
        [=](T i) { return i != 0; },
        [=](T i) { return std::make_pair(static_cast<T>(value - i), i / 2); });
}

}

template<typename T>
class Arbitrary : public Generator<T>
{
public:
    T operator()() const override
    { return detail::defaultGenerate<T>(); }

    ShrinkIteratorUP<T> shrink(T value) const override
    { return detail::defaultShrink<T>(std::move(value)); }
};

template<>
class Arbitrary<bool> : public Generator<bool>
{
public:
    bool operator()() const override
    { return (pick(resize(kReferenceSize, arbitrary<uint8_t>())) & 0x1) == 0; }
};

template<typename T1, typename T2>
class Arbitrary<std::pair<T1, T2>> : public Generator<std::pair<T1, T2>>
{
public:
    std::pair<T1, T2> operator()() const override
    { return std::make_pair(pick<T1>(), pick<T2>()); }

    ShrinkIteratorUP<std::pair<T1, T2>>
    shrink(std::pair<T1, T2> pair) const override
    {
        return sequentially(
            mapShrink(arbitrary<T1>().shrink(pair.first),
                      [=](T1 x) { return std::make_pair(x, pair.second); }),
            mapShrink(arbitrary<T2>().shrink(pair.second),
                      [=](T2 x) { return std::make_pair(pair.first, x); }));
    }
};

template<typename Coll, typename ValueType>
class ArbitraryCollection : public Generator<Coll>
{
public:
    Coll operator()() const override
    { return pick(collection<Coll>(arbitrary<ValueType>())); }

    ShrinkIteratorUP<Coll> shrink(Coll value) const override
    { return collection<Coll>(arbitrary<ValueType>()).shrink(value); }
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
    : public Generator<std::basic_string<T, Traits, Alloc>>
{
public:
    typedef std::basic_string<T, Traits, Alloc> StringType;

    StringType operator()() const override
    { return pick(collection<StringType>(character<T>())); }

    ShrinkIteratorUP<StringType> shrink(StringType value) const override
    { return collection<StringType>(character<T>()).shrink(value); }
};

}
