#pragma once

#include "Arbitrary.hpp"

namespace rc {

//Forward declaration
template<typename T> struct Arbitrary;

//! Returns a generator for arbitrary values of type T. Simply a wrapper around
//! Arbitrary for naming consistency.
template<typename T>
Arbitrary<T> arbitrary() { return Arbitrary<T>(); }

//! Implementation of the generator returned by suchThat. However, the suchThat
//! function is more convenient so it should be preferred.
template<typename Generator, typename Predicate>
class SuchThat
{
public:
    typedef typename std::result_of<Generator(size_t)>::type ResultType;

    SuchThat(const Generator &generator, const Predicate &predicate)
        : m_generator(generator), m_predicate(predicate) {}

    ResultType operator()(size_t size)
    {
        ResultType x = m_generator(size);
        while (!m_predicate(x))
            x = m_generator(size);
        return x;
    }

private:
    Generator m_generator;
    Predicate m_predicate;
};

//! Uses another generator to generate values satisfying a given condition.
//!
//! @param gen   The underlying generator to use.
//! @param pred  The predicate that the generated values must satisfy
template<typename Generator, typename Predicate>
SuchThat<Generator, Predicate> suchThat(const Generator &gen,
                                        const Predicate &pred)
{
    return SuchThat<Generator, Predicate>(gen, pred);
}

}
