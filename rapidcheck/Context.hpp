#pragma once

#include <cstdint>
#include <random>

namespace rc {

//! The integral type returned by basic random function in rapidcheck.
typedef uint64_t BasicInt;

//! The real type returned by basic random functions in rapidcheck.
typedef double BasicReal;

//! Keeps track of test parameters during checking.
class Context
{
public:
    //! Returns the instance.
    static Context &instance()
    {
        return s_instance;
    }

    //! Returns a random integer.
    BasicInt getRandomInt()
    {
        return m_intDistribution(m_randomEngine);
    }

    //! Returns a random real between in the range [0.0, 1.0).
    BasicReal getRandomReal()
    {
        return m_realDistribution(m_randomEngine);
    }

private:
    Context()
        : m_randomEngine(std::random_device()())
    {
    }

    // Non-copyable
    Context &operator=(const Context &) = delete;
    Context(const Context &) = delete;

    std::default_random_engine m_randomEngine;
    std::uniform_int_distribution<uint64_t> m_intDistribution;
    std::uniform_real_distribution<double> m_realDistribution;

    static Context s_instance;
};

Context Context::s_instance;

}
