#include "rapidcheck/detail/Suite.h"

#include "Results.h"

namespace rc {
namespace detail {

TestResults Property::run(TestDelegate &delegate) const
{
    delegate.onPropertyStart(*this);
    TestResults results(doRun(delegate));
    delegate.onPropertyFinished(*this, results);
    return results;
}

std::string Property::description() const { return m_description; }
const PropertyParams &Property::params() const { return m_params; }

TestResults Property::doRun(TestDelegate &delegate) const
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

bool Property::runCase(const TestCase &testCase) const
{
    return withTestCase(testCase, [this]{
        return (*m_generator)();
    });
}

TestResults Property::shrinkFailingCase(const TestCase &testCase) const
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
auto Property::withTestCase(const TestCase &testCase, Callable callable) const
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

TestGroup::TestGroup(std::string description)
    : m_description(description) {}

void TestGroup::add(Property &&property)
{ m_properties.push_back(std::move(property)); }

void TestGroup::run(TestDelegate &delegate)
{
    delegate.onGroupStart(*this);
    for (auto &property : m_properties)
        property.run(delegate);
    delegate.onGroupFinished(*this);
}

int TestGroup::count() const
{ return m_properties.size(); }

std::string TestGroup::description() const { return m_description; }

void TestSuite::add(TestGroup &&group)
{ m_groups.push_back(std::move(group)); }

void TestSuite::run(TestDelegate &delegate)
{
    delegate.onSuiteStart(*this);
    for (auto &group : m_groups)
        group.run(delegate);
    delegate.onSuiteFinished(*this);
}

int TestSuite::count() const
{
    int sum = 0;
    for (const auto &group : m_groups)
        sum += group.count();
    return sum;
}

TestSuite &TestSuite::defaultSuite()
{
    static TestSuite suite;
    return suite;
}

} // namespace detail
} // namespace rc
