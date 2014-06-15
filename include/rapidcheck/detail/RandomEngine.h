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
    Atom nextAtom();

    //! Sets the seed of this random engine.
    void seed(Atom s);

private:
    std::default_random_engine m_randomEngine;
    std::uniform_int_distribution<uint64_t> m_distribution;
};

namespace param {

} // namespace param

} // namespace detail
} // namespace rc
