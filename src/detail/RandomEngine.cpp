#include "rapidcheck/detail/RandomEngine.h"

#include <cassert>

#include "rapidcheck/detail/Utility.h"

namespace rc {
namespace detail {

RandomEngine::RandomEngine(Seed seed)
{
    // This is sort of a hack, but since neither avalanche() nor Xorshift128+
    // works with zeroes and we still want 0 to be a valid seed, we'll treat it
    // in a different way.
    if (seed == 0) {
        seed = 1337;
        // Reverse, so that it's not the same as the seed 1337
        m_state[1] = avalanche(seed);
        m_state[0] = avalanche(m_state[1]);
    } else {
        m_state[0] = avalanche(seed);
        m_state[1] = avalanche(m_state[0]);
    }
}

RandomEngine::Atom RandomEngine::nextAtom()
{
    uint64_t s1 = m_state[0];
    const uint64_t s0 = m_state[1];
    m_state[0] = s0;
    s1 ^= s1 << 23; // a
    return (m_state[1] = (s1 ^ s0 ^ (s1 >> 17 ) ^ (s0 >> 26 ))) + s0; // b, c
}

} // namespace detail
} // namespace rc
