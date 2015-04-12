#pragma once

#include "rapidcheck/detail/BitStream.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/newgen/Transform.h"

namespace rc {
namespace newgen {
namespace detail {

template<typename T>
Shrinkable<T> integral(const Random &random, int size)
{
    return shrinkable::shrinkRecur(
        rc::detail::bitStreamOf(random).nextWithSize<T>(size),
        &shrink::integral<T>);
}

template<typename T>
Shrinkable<T> real(const Random &random, int size)
{
    // TODO this implementation sucks
    auto stream = rc::detail::bitStreamOf(random);
    double scale =
        std::min(size, gen::kNominalSize) /
        static_cast<double>(gen::kNominalSize);
    double a = stream.nextWithSize<int64_t>(size);
    double b =
        (stream.next<uint64_t>() * scale) /
        std::numeric_limits<uint64_t>::max();
    T value = a + b;
    return shrinkable::shrinkRecur(value, &shrink::real<T>);
}

template<typename T>
struct DefaultArbitrary
{
    // If you ended up here, it means that RapidCheck wanted to generate an
    // arbitrary value of some type but you haven't declared a specialization of
    // NewArbitrary for your type. Check the template stack trace to see which type it is.
    static_assert(
        std::is_integral<T>::value,
        "No NewArbitrary specialization for type T");

    static Gen<T> arbitrary() { return integral<T>; }
};

template<>
struct DefaultArbitrary<float>
{
    static Gen<float> arbitrary() { return real<float>; }
};

template<>
struct DefaultArbitrary<double>
{
    static Gen<double> arbitrary() { return real<double>; }
};

template<>
struct DefaultArbitrary<bool>
{
    static Gen<bool> arbitrary()
    {
        return [](const Random &random, int size) {
            return shrinkable::shrinkRecur(
                rc::detail::bitStreamOf(random).next<bool>(),
                &shrink::boolean);
        };
    }
};

} // namespace detail

template<typename T>
Gen<T> inRange(T min, T max)
{
    return [=](const Random &random, int size) {
        if (max <= min) {
            std::string msg;
            msg += "Invalid range [" + std::to_string(min);
            msg += ", " + std::to_string(max) + ")";
            throw GenerationFailure(msg);
        }

        const auto rangeSize = Random::Number(max - min);
        const auto x = Random(random).next() % rangeSize;
        return shrinkable::just<T>(static_cast<T>(x) + min);
    };
}


template<typename T>
Gen<T> nonZero()
{
    return newgen::suchThat(newgen::arbitrary<T>(),
                            [](T x) { return x != 0; });
}

template<typename T>
Gen<T> positive()
{
    return newgen::suchThat(newgen::arbitrary<T>(),
                            [](T x) { return x > 0; });
}

template<typename T>
Gen<T> negative()
{
    return newgen::suchThat(newgen::arbitrary<T>(),
                            [](T x) { return x < 0; });
}

template<typename T>
Gen<T> nonNegative()
{
    return newgen::suchThat(newgen::arbitrary<T>(),
                            [](T x) { return x >= 0; });
}

} // namespace newgen
} // namespace rc
