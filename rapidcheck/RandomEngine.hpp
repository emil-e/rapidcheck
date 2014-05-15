#pragma once

#include <random>

namespace rc {
namespace detail {

//! The fundamental source of randomness in rapidcheck. Generates the most basic
//! random values which are then used to generate more complex structures.
class RandomEngine
{
public:
    //! The basic integral type upon which all other rapicheck generators are
    //! built. All other generates are compositions or transformation of one
    //! or more values of \c Atom.
    typedef uint64_t Atom;

    //! Default constructor
    RandomEngine() = default;

    //! Returns the next \c Atom.
    Atom nextAtom()
    {
        return m_distribution(m_randomEngine);
    }

    //! Resets this random engine.
    void reset()
    {
        // TODO what should be reset?
        //m_randomEngine.reset();
        m_distribution.reset();
    }

private:
    std::default_random_engine m_randomEngine;
    std::uniform_int_distribution<uint64_t> m_distribution;
};

namespace param {

//! The current random engine.
struct RandomEngine { typedef rc::detail::RandomEngine ValueType; };

} // namespace param

} // namespace detail
} // namespace rc
