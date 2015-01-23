#include "rapidcheck/detail/RandomEngine.h"

namespace rc {
namespace detail {

RandomEngine::Atom RandomEngine::nextAtom()
{
    return m_distribution(m_randomEngine);
}

void RandomEngine::seed(RandomEngine::Atom s)
{
    m_randomEngine.seed(s);
    m_distribution.reset();
}

} // namespace detail
} // namespace rc
