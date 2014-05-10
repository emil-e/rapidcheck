#pragma once

namespace rc {
namespace detail {

class RandomEngine
{
public:
    //! The integral type returned by basic random function in rapidcheck.
    typedef uint64_t Int;

    //! The real type returned by basic random functions in rapidcheck.
    typedef double Real;

    //! Default constructor
    RandomEngine() = default;

    //! Returns a random integer.
    Int getRandomInt()
    {
        return m_intDistribution(m_randomEngine);
    }

    //! Returns a random real between in the range [0.0, 1.0).
    Real getRandomReal()
    {
        return m_realDistribution(m_randomEngine);
    }

    //! Resets this random engine.
    void reset()
    {
        // TODO what should be reset?
        //m_randomEngine.reset();
        m_intDistribution.reset();
        m_realDistribution.reset();
    }

private:
    // Non-copyable
    RandomEngine &operator=(const RandomEngine &) = delete;
    RandomEngine(const RandomEngine &) = delete;

    std::default_random_engine m_randomEngine;
    std::uniform_int_distribution<uint64_t> m_intDistribution;
    std::uniform_real_distribution<double> m_realDistribution;
};

}
}
