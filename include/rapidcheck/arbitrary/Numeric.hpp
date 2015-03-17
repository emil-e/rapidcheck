#pragma once

#include "rapidcheck/gen/Generator.h"
#include "rapidcheck/gen/Parameters.h"
#include "rapidcheck/shrink/Shrink.h"

namespace rc {

template<typename T>
class Arbitrary : public gen::Generator<T>
{
public:
    static_assert(std::is_integral<T>::value,
                  "No specialization of Arbitrary for type");

    T generate() const override
    {
        using namespace detail;

        int size = std::min(gen::currentSize(), gen::kNominalSize);
        Random::Number r = ImplicitParam<param::Random>::value().next();

        // We vary the size by using different number of bits. This way, we can
        // be sure that the max value can also be generated.
        int nBits = (size * std::numeric_limits<T>::digits) / gen::kNominalSize;
        if (nBits == 0)
            return 0;
        constexpr auto randIntMax = std::numeric_limits<Random::Number>::max();
        Random::Number mask = ~((randIntMax - 1) << (nBits - 1));

        T x = static_cast<T>(r & mask);
        if (std::numeric_limits<T>::is_signed)
        {
            // Use the topmost bit as the signed bit. Even in the case of a
            // signed 64-bit integer, it won't be used since it actually IS the
            // sign bit.
            constexpr int basicBits =
                std::numeric_limits<Random::Number>::digits;
            x *= ((r >> (basicBits - 1)) == 0) ? 1 : -1;
        }

        return x;
    }

    Seq<T> shrink(T value) const override
    { return shrink::integral(value); }
};

// Base for float and double arbitrary instances
template<typename T>
class ArbitraryReal : public gen::Generator<T>
{
public:
    T generate() const override
    {
        int64_t i = *gen::arbitrary<int64_t>();
        T x = static_cast<T>(i) / std::numeric_limits<int64_t>::max();
        return std::pow<T>(1.1, gen::currentSize()) * x;
    }

    Seq<T> shrink(T value) const override
    { return shrink::real(value); }
};

template<>
class Arbitrary<float> : public ArbitraryReal<float> {};

template<>
class Arbitrary<double> : public ArbitraryReal<double> {};

template<>
class Arbitrary<bool> : public gen::Generator<bool>
{
public:
    bool generate() const override
    {
        return (*gen::resize(gen::kNominalSize,
                                 gen::arbitrary<uint8_t>()) & 0x1) == 0;
    }

    Seq<bool> shrink(bool value) const override
    { return shrink::boolean(value); }
};

} // namespace rc
