#pragma once

#include "AnsiEscape.hpp"

namespace rc {
namespace detail {

//! `TestDelegate` implementation for printing to console.
class ConsoleDelegate : public TestDelegate
{
public:
    //! Constructors
    //!
    //! @param output  The output stream to print to.
    explicit ConsoleDelegate(std::ostream &output) : m_output(output) {}

    void onSuiteStart(const TestSuite &suite)
    {
        m_failures.clear();
    }

    void onGroupStart(const TestGroup &group) override
    {
        m_output << ansi::attr(ansi::AttrBold)
                 << group.description() << std::endl;
    }

    void onPropertyStart(const Property &prop) override
    {
        m_output << ansi::attr(ansi::AttrNone)
                 << " - " << prop.description() << std::endl;
    }

    void onPropertyTestCase(const Property &prop, const TestCase &testCase) override
    {
        m_output << ansi::cursorHome << ansi::eraseLine
                 << indent << testCase.index << "/" << prop.params().maxSuccess
                 << std::flush;
    }

    void onShrinkStart(const Property &prop,
                       const TestCase &testCase) override
    {
        m_output << ansi::cursorUp(1) << ansi::cursorHome << ansi::eraseLine
                 << " - "
                 << ansi::attr(ansi::FgRed) << prop.description() << std::endl
                 << ansi::attr(ansi::AttrNone) << ansi::eraseLine
                 << testCase.index << "/" << prop.params().maxSuccess
                 << " failed! Shrinking..." << std::flush;
    }

    void onPropertyFinished(const Property &prop,
                            const TestResults &results) override
    {
        if (results.result == Result::Success)
            onPropertySuccess(prop, results);
        else
            onPropertyFailure(prop, results);
    }

    //! Called when a group finishes
    void onGroupFinished(const TestGroup &group) override
    {
        m_output << std::endl;
    }

    void onSuiteFinished(const TestSuite &suite)
    {
        for (auto it = m_failures.begin(); it != m_failures.end(); it++) {
            int index = it - m_failures.begin() + 1;
            m_output << ansi::attr(ansi::AttrBold) << index << ") "
                     << ansi::attr(ansi::FgRed) << it->first << ":"
                     << ansi::attr(ansi::AttrNone) << std::endl;
            printCounterExample(it->second);
        }
    }

private:
    void onPropertyFailure(const Property &prop, const TestResults &results)
    {
        m_failures.emplace_back(prop.description(), results.counterExample);

        m_output << ansi::cursorHome << ansi::eraseLine << indent
                 << "Falsifiable, after " << results.failingCase.index
                 << " tests";
        if (results.numShrinks > 0) {
            m_output << " and " << results.numShrinks;
            m_output << (results.numShrinks > 1 ? " shrinks" : " shrink");
        }
        m_output << ansi::attr(ansi::AttrBold)
                 << " (" << m_failures.size() << ")"
                 << ansi::attr(ansi::AttrNone) << std::endl;
    }

    void onPropertySuccess(const Property &prop, const TestResults &results)
    {
        m_output << ansi::cursorUp(1) << ansi::cursorHome << ansi::eraseLine
                 << " - "
                 << ansi::attr(ansi::FgGreen) << prop.description() << std::endl
                 << ansi::attr(ansi::AttrNone) << ansi::eraseLine
                 << indent << "OK, passed " << prop.params().maxSuccess
                 << " tests" << std::endl;
    }

    void printCounterExample(const std::vector<gen::ValueDescription> &example)
    {
        for (const auto &desc : example) {
            m_output << ansi::attr(ansi::FgBlue)
                     << desc.typeName() << ":" << std::endl
                     << ansi::attr(ansi::AttrNone)
                     << desc.stringValue() << std::endl << std::endl;
        }
    }

    static constexpr const char *indent = "    ";

    std::ostream &m_output;
    typedef std::vector<gen::ValueDescription> CounterExample;
    typedef std::pair<std::string, CounterExample> Failure;
    std::vector<Failure> m_failures;
};

} // namespace detail
} // namespace rc
