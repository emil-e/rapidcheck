#pragma once

#include <cstddef>
#include <string>

#include "rapidcheck/Generator.h"

namespace rc {
namespace detail {

class Property;
class TestGroup;
class TestSuite;
class TestCase;
class TestResults;

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
    TestResults run(TestDelegate &delegate) const;

    //! Returns the description.
    std::string description() const;

    //! Returns the test parameters.
    const PropertyParams &params() const;

private:
    TestResults doRun(TestDelegate &delegate) const;

    bool runCase(const TestCase &testCase) const;

    TestResults shrinkFailingCase(const TestCase &testCase) const;

    template<typename Callable>
    auto withTestCase(const TestCase &testCase, Callable callable) const
        -> decltype(callable());

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
    explicit TestGroup(std::string description);

    //! Adds a `Property` to this group.
    void add(Property &&property);

    //! Runs this group
    //!
    //! @param delegate  The delegate that should receive test running events.
    void run(TestDelegate &delegate);

    //! Returns the number of tests in this group.
    int count() const;

    //! Returns the description.
    std::string description() const;

private:
    std::string m_description;
    std::vector<Property> m_properties;
};


//! Groups together `Property`s and associates metadata
class TestSuite
{
public:
    //! Adds a `Property` to this suite.
    void add(TestGroup &&group);

    //! Runs the suite
    //!
    //! @param delegate  The delegate that should receive test running events.
    void run(TestDelegate &delegate);

    //! Returns the total number of tests in this suite.
    int count() const;

    //! Returns the default `TestSuite` instance.
    static TestSuite &defaultSuite();

private:
    std::string m_description;
    std::vector<TestGroup> m_groups;
};

} // namespace detail
} // namespace rc
