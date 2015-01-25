#pragma once

#include <array>
#include <cstdint>

namespace rc {
namespace detail {

//! The fundamental source of randomness in RapidCheck. Generates the most basic
//! random values which are then used to generate more complex structures.
//!
//! Currently uses Xorshift128+ from Sebastiano Vigna (vigna@acm.org). Creds to
//! him.
class RandomEngine
{
public:
    //! The basic integral type upon which all other rapicheck generators are
    //! built. All other generates are compositions or transformation of one
    //! or more values of \c Atom.
    typedef uint64_t Atom;

    //! The type of the seed.
    typedef uint64_t Seed;

    //! C-tor.
    explicit RandomEngine(Seed seed);

    //! Returns the next \c Atom. Mutates the state.
    Atom nextAtom();

private:
    uint64_t m_state[2];
};

} // namespace detail
} // namespace rc
