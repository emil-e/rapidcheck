#pragma once

#include "Generators.hpp"
#include "RandomEngine.hpp"
#include "Value.hpp"

namespace rc {

template<typename Generator>
GeneratedType<Generator> pick(const Generator &gen);

template<typename T> T pick();

namespace detail {

//! The integral type returned by basic random function in rapidcheck.
typedef uint64_t BasicInt;

//! The real type returned by basic random functions in rapidcheck.
typedef double BasicReal;

//! This is the class that does the actual testing. It is NOT thread safe!!!
class Checker
{
public:
    //! The type of a test case, i.e. a set of parameters.
    typedef std::vector<ValueUP> TestCase;

    //! Picks a random value using the given generator and records it to the
    //! current example.
    //!
    //! @return  The generated value.
    template<typename Generator>
    GeneratedType<Generator> pick(const Generator &generator)
    {
        typedef GeneratedType<Generator> T;
        T value(generator(m_currentSize));
        m_currentTestCase.emplace_back(new StoredValue<T>(value));
        return value;
    }

    //! Returns the \c RandomEngine.
    RandomEngine &randomEngine()
    { return m_randomEngine; }

    //! Returns the instance.
    static Checker &instance()
    {
        static Checker theInstance;
        return theInstance;
    }

    //! Runs the checker on the given property.
    //!
    //! @param property  A property to test. Should be callable and return a
    //!                  bool.
    template<typename Property>
    bool check(Property property)
    {
        for (int testNumber = 1; testNumber <= m_maxSuccess; testNumber++) {
            resetTestCase();
            *m_output << testNumber << "/" << m_maxSuccess << std::endl;
            if (!property()) {
                onFailure(testNumber, m_currentTestCase);
                return false;
            }
            printTestCase(m_currentTestCase);
            m_currentSize = std::min(m_maxSize, m_currentSize + 1);
        }

        return true;
    }

private:
    Checker() { reset(); }

    // Reset parameters before each test case.
    void resetTestCase()
    {
        m_currentTestCase.clear();
    }

    // Run when a test fails.
    void onFailure(int testNumber, const TestCase &failingCase)
    {
        *m_output << "Falsifiable after " << testNumber << " tests:" << std::endl;
        printTestCase(failingCase);
    }

    void printTestCase(const TestCase &testCase)
    {
        for (const auto &value : testCase) {
            value->show(*m_output);
            *m_output << std::endl;
        }
    }

    void reset()
    {
        m_currentSize = 0;
        m_currentTestCase.clear();
    }

    // Non-copyable
    Checker &operator=(const Checker &) = delete;
    Checker(const Checker &) = delete;

    RandomEngine m_randomEngine;
    size_t m_currentSize;
    TestCase m_currentTestCase;

    // Parameters
    size_t m_maxSuccess = 100;
    size_t m_maxSize = 100;
    std::ostream *m_output = &std::cout;
};

//! Helper to extract arguments as a parameter pack.
//!
//! @param MemberFuncPtr  Type of the pointer to \c operator().
template<typename MemberFuncPtr> class FunctorHelper;

// Specialize to make argument deduction given us the parameter pack.
template<typename Functor, typename ...Args>
class FunctorHelper<bool (Functor::*)(Args...) const>
{
public:
    FunctorHelper(Functor functor) : m_functor(std::move(functor)) {}

    bool operator()() const
    { return m_functor(pick<typename std::decay<Args>::type>()...); }

private:
    Functor m_functor;
};

//! A \c Quantifier calls an underlying callable with random parameter picked
//! the \c pick<T>() function when invoked through operator().
//!
//! Please note that the base template is for functors, function pointers are
//! implemented through specialization of this template.
//!
//! @param Callable  The type of the callable.
template<typename Callable>
class Quantifier
{
public:
    Quantifier(Callable callable) : m_helper(std::move(callable)) {}
    bool operator()() { return m_helper(); }

private:
    FunctorHelper<decltype(&Callable::operator())> m_helper;
};

// TODO support function pointers as well

} // namespace detail

template<typename Generator>
GeneratedType<Generator> pick(const Generator &generator)
{
    return detail::Checker::instance().pick(generator);
}

template<typename T>
T pick()
{
    return detail::Checker::instance().pick(arbitrary<T>());
}


template<typename Testable>
typename std::enable_if<std::is_class<Testable>::value, bool>::type
check(Testable testable)
{
    auto &checker = detail::Checker::instance();
    return checker.check(detail::Quantifier<Testable>(testable));
}

}
