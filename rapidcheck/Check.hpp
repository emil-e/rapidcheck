#pragma once

#include "Generators.hpp"
#include "RandomEngine.hpp"
#include "Value.hpp"

namespace rc {

template<typename Generator>
GeneratedType<Generator> pick(const Generator &gen);

template<typename T> T pick();

namespace detail {

typedef std::function<bool()> Property;

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
        T value(getValue(generator));
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
    //! @param property  A property to test.
    bool check(const Property &property)
    {
        m_currentProperty = property;
        m_currentTestNumber = 1;
        m_currentMinimum.clear();
        while (m_currentTestNumber <= m_maxSuccess) {
            m_currentTestCase.clear();
            if (!m_currentProperty()) {
                onFailure(std::move(m_currentTestCase));
                return false;
            }
            m_currentTestNumber++;
            m_currentSize = std::min(m_maxSize, m_currentSize + 1);
        }

        return true;
    }

private:
    Checker() { reset(); }

    // Retrieves a value to return from \c pick.
    template<typename Generator>
    GeneratedType<Generator> getValue(const Generator &generator)
    {
        typedef GeneratedType<Generator> T;
        if (m_currentMinimum.empty()) {
            return generator(m_currentSize);
        } else {
            // TODO check for incompatible types
            auto storedValue = dynamic_cast<StoredValue<T> *>(
                m_fixIterator->get());
            if (!m_didShrink && storedValue->hasNextShrink()) {
                // No other value has been shrunk and there is a possible shrink
                m_didShrink = true;
                return storedValue->nextShrink();
            } else {
                // Another value has already been shrunk or there is no possible
                // shrink
                return storedValue->get();
            }
        }
    }

    // Run when a test fails.
    void onFailure(TestCase failingCase)
    {
        *m_output << "Found failure:" << std::endl;
        printTestCase(failingCase);
        TestCase minimalCase = doShrink(std::move(failingCase));
        *m_output << "Falsifiable after " << m_currentTestNumber << " tests and "
                  << m_numShrinks << " shrinks:" << std::endl;
        printTestCase(minimalCase);
    }

    void printTestCase(const TestCase &testCase)
    {
        for (const auto &value : testCase) {
            value->show(*m_output);
            *m_output << std::endl;
        }
    }

    TestCase doShrink(TestCase startCase)
    {
        m_currentMinimum = std::move(startCase);
        while (!isLocalMinimum(m_currentMinimum)) {
            m_currentTestCase.clear();
            m_didShrink = false;
            m_fixIterator = m_currentMinimum.begin();
            if (!m_currentProperty()) {
                m_currentMinimum = std::move(m_currentTestCase);
                m_numShrinks++;
            }
        }

        return std::move(m_currentMinimum);
    }

    static bool isLocalMinimum(const TestCase &testCase)
    {
        return std::none_of(testCase.begin(),testCase.end(),
                            [](const ValueUP &value)
                            { return value->hasNextShrink(); });
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
    Property m_currentProperty;
    size_t m_currentSize;
    int m_currentTestNumber;
    TestCase m_currentTestCase;

    TestCase m_currentMinimum;
    TestCase::iterator m_fixIterator;
    int m_numShrinks;
    bool m_didShrink;

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
