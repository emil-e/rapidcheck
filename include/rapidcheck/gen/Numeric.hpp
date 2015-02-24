#pragma once

#include "Generator.h"
#include "rapidcheck/shrink/Shrink.h"

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

    Seq<T> shrink(T value) const override
    {
        return shrink::towards(value, m_min);
    }

private:
    T m_min, m_max;
};

template<typename T>
Ranged<T> ranged(T min, T max)
{
    static_assert(std::is_arithmetic<T>::value,
                  "ranged only supports arithmetic types");
    return Ranged<T>(min, max);
}

template<typename T>
SuchThat<Arbitrary<T>, predicate::Not<predicate::Equals<T>>> nonZero()
{ return gen::suchThat<T>(predicate::Not<predicate::Equals<T>>(0)); }

template<typename T>
SuchThat<Arbitrary<T>, predicate::GreaterThan<T>> positive()
{ return gen::suchThat<T>(predicate::GreaterThan<T>(0)); }

template<typename T>
SuchThat<Arbitrary<T>, predicate::LessThan<T>> negative()
{
    static_assert(std::is_signed<T>::value,
                  "gen::negative can only be used for signed types");
    return gen::suchThat<T>(predicate::LessThan<T>(0));
}

template<typename T>
SuchThat<Arbitrary<T>, predicate::GreaterEqThan<T>> nonNegative()
{ return gen::suchThat<T>(predicate::GreaterEqThan<T>(0)); }

} // namespace gen
} // namespace rc
