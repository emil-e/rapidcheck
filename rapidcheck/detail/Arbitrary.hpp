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
    RandomEngine::Atom r = RoseNode::current().atom();

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
    return ShrinkIteratorUP<T>(new DivideByTwoIterator<T>(value));
}

}

template<typename T>
class Arbitrary : public Generator<T>
{
public:
    T operator()() const override
    { return detail::defaultGenerate<T>(); }

    ShrinkIteratorUP<T> shrink(const T &value) const override
    { return detail::defaultShrink<T>(value); }
};

template<>
class Arbitrary<bool> : public Generator<bool>
{
public:
    bool operator()() const override
    { return (pick(resize(kReferenceSize, arbitrary<uint8_t>())) & 0x1) == 0; }
};

// std::vector
template<typename T, typename Alloc>
class Arbitrary<std::vector<T, Alloc>>
    : public Generator<std::vector<T, Alloc>>
{
public:
    typedef std::vector<T, Alloc> VectorType;

    VectorType operator()() const override
    { return pick(collection<std::vector<T, Alloc>>(arbitrary<T>())); }
};

// std::basic_string
template<typename T, typename Traits, typename Alloc>
class Arbitrary<std::basic_string<T, Traits, Alloc>>
    : Generator<std::basic_string<T, Traits, Alloc>>
{
public:
    typedef std::basic_string<T, Traits, Alloc> StringType;

    StringType operator()() const override
    {
        auto charGen = oneOf(ranged<uint8_t>(1, 127), nonZero<T>());
        return pick(collection<std::string>(charGen));
    }
};

}
