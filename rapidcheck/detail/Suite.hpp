#pragma once

#include "Results.hpp"

namespace rc {
namespace detail {

class TestSuite;
class TestGroup;
class Property;

//! Describes the parameters for a test.
struct PropertyParams
{
    //! The maximum number of successes before deciding a property passes.
    int maxSuccess = 100;
    //! The maximum size to generate.
    size_t maxSize = 100;
};

//! Handles test progress events during test running.
class TestDelegate
{
public:
    //! Called when the suite starts.
    virtual void onSuiteStart(const TestSuite &suite) = 0;

    //! Called when a test group starts running.
    virtual void onGroupStart(const TestGroup &group) = 0;

    //! Called when a property starts running.
    virtual void onPropertyStart(const Property &prop) = 0;

    //! Called after a particular test case has run.
    virtual void onPropertyTestCase(const Property &prop, const TestCase &testCase) = 0;

    //! Called on test case failure before shrinking starts.
    virtual void onShrinkStart(const Property &prop,
                               const TestCase &testCase) = 0;

    //! Called when a property finishes.
    virtual void onPropertyFinished(const Property &prop,
                                    const TestResults &results) = 0;

    //! Called when a group finishes.
    virtual void onGroupFinished(const TestGroup &group) = 0;

    //! Called when the suite ends.
    virtual void onSuiteFinished(const TestSuite &suite) = 0;

    virtual ~TestDelegate() = default;
};

//! Associates metadata with a generator of type bool
class Property
{
public:
    //! Constructor
    //!
    //! @param description  A description of the property.
    //! @param generator    The generator that implements the property.
    //! @param params       The parameters to use
    template<typename Gen>
    Property(std::string description, Gen generator, PropertyParams params)
        : m_description(std::move(description))
        , m_generator(gen::GeneratorUP<typename Gen::GeneratedType>(
                          new Gen(std::move(generator))))
        , m_params(params) {}

    //! Runs this property with the given `PropertyParams`.
    TestResults run(TestDelegate &delegate) const
    {
        delegate.onPropertyStart(*this);
        TestResults results(doRun(delegate));
        delegate.onPropertyFinished(*this, results);
        return results;
    }

    //! Returns the description.
    std::string description() const { return m_description; }

    //! Returns the test parameters.
    const PropertyParams &params() const { return m_params; }

private:
    TestResults doRun(TestDelegate &delegate) const
    {
        using namespace detail;
        TestCase currentCase;
        RandomEngine seedEngine;

        currentCase.size = 0;
        for (currentCase.index = 1;
             currentCase.index <= m_params.maxSuccess;
             currentCase.index++)
        {
            currentCase.seed = seedEngine.nextAtom();

            bool success = runCase(currentCase);
            delegate.onPropertyTestCase(*this, currentCase);
            if (!success) {
                delegate.onShrinkStart(*this, currentCase);
                return shrinkFailingCase(currentCase);
            }

            currentCase.size = std::min(m_params.maxSize, currentCase.size + 1);
            //TODO better size calculation
        }

        return TestResults();
    }

    bool runCase(const TestCase &testCase) const
    {
        return withTestCase(testCase, [this]{
            return (*m_generator)();
        });
    }

    TestResults shrinkFailingCase(const TestCase &testCase) const
    {
        return withTestCase(testCase, [&testCase, this]{
            RoseNode rootNode;
            TestResults results;
            results.result = Result::Failure;
            results.failingCase = testCase;
            results.numShrinks = rootNode.shrink(*m_generator);
            results.counterExample = rootNode.example();
            return results;
        });
    }

    template<typename Callable>
    auto withTestCase(const TestCase &testCase, Callable callable) const
        -> decltype(callable())
    {
        ImplicitParam<param::RandomEngine> randomEngine;
        randomEngine.let(RandomEngine());
        randomEngine->seed(testCase.seed);
        ImplicitParam<param::Size> size;
        size.let(testCase.size);
        ImplicitParam<param::NoShrink> noShrink;
        noShrink.let(false);

        return callable();
    }

    std::string m_description;
    gen::GeneratorUP<bool> m_generator;
    PropertyParams m_params;
};


//! Groups together `Property`s and associates metadata
class TestGroup
{
public:
    //! Constructor
    //!
    //! @param description  A description of the group.
    explicit TestGroup(std::string description)
        : m_description(description) {}

    //! Adds a `Property` to this group.
    void add(Property &&property)
    { m_properties.push_back(std::move(property)); }

    //! Runs this group
    //!
    //! @param delegate  The delegate that should receive test running events.
    void run(TestDelegate &delegate)
    {
        delegate.onGroupStart(*this);
        for (auto &property : m_properties)
            property.run(delegate);
        delegate.onGroupFinished(*this);
    }

    //! Returns the number of tests in this group.
    int count() const
    { return m_properties.size(); }

    //! Returns the description.
    std::string description() const { return m_description; }

private:
    std::string m_description;
    std::vector<Property> m_properties;
};


//! Groups together `Property`s and associates metadata
class TestSuite
{
public:
    //! Constructor
    //!
    //! @param description  A description of the suite.
    explicit TestSuite() {}

    //! Adds a `Property` to this suite.
    void add(TestGroup &&group)
    { m_groups.push_back(std::move(group)); }

    //! Runs the suite
    //!
    //! @param delegate  The delegate that should receive test running events.
    void run(TestDelegate &delegate)
    {
        delegate.onSuiteStart(*this);
        for (auto &group : m_groups)
            group.run(delegate);
        delegate.onSuiteFinished(*this);
    }

    //! Returns the total number of tests in this suite.
    int count() const
    {
        int sum = 0;
        for (const auto &group : m_groups)
            sum += group.count();
        return sum;
    }

    //! Returns the default `TestSuite` instance.
    static TestSuite &defaultSuite()
    {
        static TestSuite suite;
        return suite;
    }

private:
    std::string m_description;
    std::vector<TestGroup> m_groups;
};


} // namespace detail
} // namespace rc
