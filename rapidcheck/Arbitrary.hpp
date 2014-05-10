#pragma once

#include <limits>
#include <type_traits>

#include "Context.hpp"
#include "Generators.hpp"
#include "Check.hpp"

namespace rc {

template<typename T> struct Arbitrary;

namespace detail {

// Signed integer generation
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
defaultGenerate(size_t size)
{
    size = std::min(size, kReferenceSize);

    auto &randomEngine = detail::Checker::instance().randomEngine();
    RandomEngine::Int r = randomEngine.getRandomInt();

    // We vary the size by using different number of bits. This way, we can be
    // that the max value can also be generated.
    int nBits = (size * std::numeric_limits<T>::digits) / kReferenceSize;
    if (nBits == 0)
        return 0;
    constexpr RandomEngine::Int randIntMax =
        std::numeric_limits<RandomEngine::Int>::max();
    BasicInt mask = ~((randIntMax - 1) << (nBits - 1));

    T x = static_cast<T>(r & mask);
    if (std::numeric_limits<T>::is_signed)
    {
        // Use the topmost bit as the signed bit. Even in the case of a signed
        // 64-bit integer, it won't be used since it actually IS the sign bit.
        constexpr int basicBits = std::numeric_limits<RandomEngine::Int>::digits;
        x *= ((r >> (basicBits - 1)) == 0) ? 1 : -1;
    }

    return x;
}

// Real generation
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
defaultGenerate(size_t size)
{
    //TODO implement sizing
    auto &randomEngine = detail::Checker::instance().randomEngine();
    return static_cast<T>(randomEngine.getRandomReal());
}

}

//! Template for generators of arbitrary values of different types. Specialize
//! template to provide generation for custom types.
//!
//! @tparam T       The type to generate.
//! @tparam Enable  To be used with \c enable_if
template<typename T>
struct Arbitrary
{
    //! Generates a value of type T.
    //!
    //! @param size  The "size" of the value to generate. This can mean
    //!              different things for different types. For a container
    //!              class, this can mean the size of the container, for
    //!              example.
    //!
    //! @return The generated value.
    T operator()(size_t size) const { return detail::defaultGenerate<T>(size); }
};

template<>
struct Arbitrary<bool>
{
    bool operator()(size_t size) const
    { return (arbitrary<uint8_t>()(kReferenceSize) & 0x1) == 0; }
};

// std::vector
template<typename T, typename Alloc>
struct Arbitrary<std::vector<T, Alloc>>
{
    typedef std::vector<T, Alloc> VectorType;

    VectorType operator()(size_t size) const
    { return collection<std::vector<T, Alloc>>(arbitrary<T>())(size); }
};

// std::basic_string
template<typename T, typename Traits, typename Alloc>
struct Arbitrary<std::basic_string<T, Traits, Alloc>>
{
    typedef std::basic_string<T, Traits, Alloc> StringType;

    StringType operator()(size_t size) const
    {
        auto charGen = resize(kReferenceSize,
                              oneOf(ranged<uint8_t>(1, 127), nonZero<T>()));
        return collection<std::string>(charGen)(size);
    }
};

}
