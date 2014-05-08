#pragma once

#include <limits>
#include <type_traits>

#include "Context.hpp"
#include "Generators.hpp"

namespace rc {

template<typename T> struct Arbitrary;

template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
defaultGenerate(size_t size)
{
    auto &context = Context::instance();

    BasicInt r = context.getRandomInt();
    int nBits = (size * std::numeric_limits<T>::digits) / 100;
    BasicInt mask = ~(std::numeric_limits<BasicInt>::max() << nBits);
    T x = static_cast<T>(r & mask);
    if (std::numeric_limits<T>::is_signed)
    {
        // Use the topmost bit as the signed bit. Even in the case of a signed
        // 64-bit integer, it won't be used since it actually IS the sign bit.
        x *= ((r >> (std::numeric_limits<BasicInt>::digits - 1)) == 0) ? 1 : -1;
    }

    return x;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
defaultGenerate(size_t size)
{
    //TODO implement sizing
    auto &context = Context::instance();
    return static_cast<T>(context.getRandomReal());
}

//! Template for generators of arbitrary values of different types. Specialize
//! template to provide generation for custom types.
//!
//! @tparam T       The type to generate.
//! @tparam Enable  To be used with enable_if
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
    T operator()(size_t size) const { return defaultGenerate<T>(size); }
};

template<>
struct Arbitrary<bool>
{
    bool operator()(size_t size) const
    {
        return (Arbitrary<uint8_t>()(100) & 0x1) == 0;
    }
};

template<typename T, typename Alloc>
struct Arbitrary<std::vector<T, Alloc>>
{
    typedef std::vector<T, Alloc> VectorType;

    VectorType operator()(size_t size) const
    {
        auto length = Arbitrary<typename VectorType::size_type>()(size);
        VectorType vec(length);
        std::generate(vec.begin(), vec.end(), [&]{ return Arbitrary<T>()(size); });
        return vec;
    }
};

template<typename T, typename Traits, typename Alloc>
struct Arbitrary<std::basic_string<T, Traits, Alloc>>
{
    typedef std::basic_string<T, Traits, Alloc> StringType;

    StringType operator()(size_t size) const
    {
        auto length = Arbitrary<typename StringType::size_type>()(size);
        StringType str(length, '\0');
        auto nonZeroChar = suchThat(Arbitrary<T>(), [](T x){ return x != 0; });
        std::generate(str.begin(), str.end(), [&]{ return nonZeroChar(100); });
        return str;
    }
};

}
