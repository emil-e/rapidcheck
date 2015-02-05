#pragma once

#include "Generator.h"

namespace rc {
namespace gen {

template<typename T>
class Ranged : public Generator<T>
{
public:
    Ranged(T min, T max) : m_min(min), m_max(max) {}

    T generate() const override
    {
        if (m_max < m_min) {
            std::string msg;
            msg += "Invalid range [" + std::to_string(m_min);
            msg += ", " + std::to_string(m_max) + ")";
            throw GenerationFailure(msg);
        }

        if (m_max == m_min)
            return m_max;

        // TODO this seems a bit broken
        typedef typename std::make_unsigned<T>::type Uint;
        Uint value(*noShrink(resize(kNominalSize, arbitrary<Uint>())));
        return m_min + value % (m_max - m_min);
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        return shrink::towards(value, m_min);
    }

private:
    T m_min, m_max;
};

// Generators of this form are common, let's not repeat ourselves
#define IMPLEMENT_SUCH_THAT_GEN(GeneratorName, predicate)               \
    template<typename T>                                                \
    class GeneratorName : public Generator<T>                           \
    {                                                                   \
    public:                                                             \
        T generate() const                                              \
        { return *suchThat<T>([](T x) { return (predicate); }); }       \
    };

IMPLEMENT_SUCH_THAT_GEN(NonZero, x != 0)
IMPLEMENT_SUCH_THAT_GEN(Positive, x > 0)
IMPLEMENT_SUCH_THAT_GEN(Negative, x < 0)
IMPLEMENT_SUCH_THAT_GEN(NonNegative, x >= 0)

#undef IMPLEMENT_SUCH_THAT_GEN

template<typename T>
Ranged<T> ranged(T min, T max)
{
    static_assert(std::is_arithmetic<T>::value,
                  "ranged only supports arithmetic types");
    return Ranged<T>(min, max);
}

template<typename T>
NonZero<T> nonZero()
{ return NonZero<T>(); }

template<typename T>
Positive<T> positive()
{ return Positive<T>(); }

template<typename T>
Negative<T> negative()
{
    static_assert(std::is_signed<T>::value,
                  "gen::negative can only be used for signed types");
    return Negative<T>();
}

template<typename T>
NonNegative<T> nonNegative()
{ return NonNegative<T>(); }

} // namespace gen
} // namespace rc
