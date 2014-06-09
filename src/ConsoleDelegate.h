#pragma once

#include "rapidcheck/detail/Suite.h"

namespace rc {
namespace detail {

//! `TestDelegate` implementation for printing to console.
class ConsoleDelegate : public TestDelegate
{
public:
    //! Constructors
    //!
    //! @param output  The output stream to print to.
    explicit ConsoleDelegate(std::ostream &output);

    void onSuiteStart(const TestSuite &suite);
    void onGroupStart(const TestGroup &group) override;
    void onTestStart(const PropertyTest &prop) override;
    void onPropertyTestCase(const PropertyTest &prop,
                            const TestCase &testCase) override;
    void onShrinkStart(const PropertyTest &prop, const TestCase &testCase) override;
    void onPropertyFinished(const PropertyTest &prop,
                            const TestResults &results) override;
    void onGroupFinished(const TestGroup &group) override;
    void onSuiteFinished(const TestSuite &suite) override;

private:
    void onPropertyFailure(const PropertyTest &prop, const TestResults &results);
    void onPropertySuccess(const PropertyTest &prop, const TestResults &results);
    void printCounterExample(const std::vector<gen::ValueDescription> &example);

    static constexpr const char *indent = "    ";

    std::ostream &m_output;
    typedef std::vector<gen::ValueDescription> CounterExample;
    typedef std::pair<std::string, CounterExample> Failure;
    std::vector<Failure> m_failures;
};

} // namespace detail
} // namespace rc
