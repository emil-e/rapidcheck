#include "ConsoleDelegate.h"

#include "rapidcheck/detail/Suite.h"
#include "Results.h"
#include "AnsiEscape.h"

namespace rc {
namespace detail {

ConsoleDelegate::ConsoleDelegate(std::ostream &output) : m_output(output) {}

void ConsoleDelegate::onSuiteStart(const TestSuite &suite)
{
    m_failures.clear();
}

void ConsoleDelegate::onGroupStart(const TestGroup &group)
{
    m_output << ansi::attr(ansi::AttrBold)
             << group.description() << std::endl;
}

void ConsoleDelegate::onPropertyStart(const Property &prop)
{
    m_output << ansi::attr(ansi::AttrNone)
             << " - " << prop.description() << std::endl;
}

void ConsoleDelegate::onPropertyTestCase(const Property &prop,
                                         const TestCase &testCase)
{
    m_output << ansi::cursorHome << ansi::eraseLine
             << indent << testCase.index << "/" << prop.params().maxSuccess
             << std::flush;
}

void ConsoleDelegate::onShrinkStart(const Property &prop,
                   const TestCase &testCase)
{
    m_output << ansi::cursorUp(1) << ansi::cursorHome << ansi::eraseLine
             << " - "
             << ansi::attr(ansi::FgRed) << prop.description() << std::endl
             << ansi::attr(ansi::AttrNone) << ansi::eraseLine
             << testCase.index << "/" << prop.params().maxSuccess
             << " failed! Shrinking..." << std::flush;
}

void ConsoleDelegate::onPropertyFinished(const Property &prop,
                        const TestResults &results)
{
    if (results.result == Result::Success)
        onPropertySuccess(prop, results);
    else
        onPropertyFailure(prop, results);
}

//! Called when a group finishes
void ConsoleDelegate::onGroupFinished(const TestGroup &group)
{
    m_output << std::endl;
}

void ConsoleDelegate::onSuiteFinished(const TestSuite &suite)
{
    for (auto it = m_failures.begin(); it != m_failures.end(); it++) {
        int index = it - m_failures.begin() + 1;
        m_output << ansi::attr(ansi::AttrBold) << index << ") "
                 << ansi::attr(ansi::FgRed) << it->first << ":"
                 << ansi::attr(ansi::AttrNone) << std::endl;
        printCounterExample(it->second);
    }
}

void ConsoleDelegate::onPropertyFailure(const Property &prop,
                                        const TestResults &results)
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

void ConsoleDelegate::onPropertySuccess(const Property &prop,
                                        const TestResults &results)
{
    m_output << ansi::cursorUp(1) << ansi::cursorHome << ansi::eraseLine
             << " - "
             << ansi::attr(ansi::FgGreen) << prop.description() << std::endl
             << ansi::attr(ansi::AttrNone) << ansi::eraseLine
             << indent << "OK, passed " << prop.params().maxSuccess
             << " tests" << std::endl;
}

void ConsoleDelegate::printCounterExample(
    const std::vector<gen::ValueDescription> &example)
{
    for (const auto &desc : example) {
        m_output << ansi::attr(ansi::FgBlue)
                 << desc.typeName() << ":" << std::endl
                 << ansi::attr(ansi::AttrNone)
                 << desc.stringValue() << std::endl << std::endl;
    }
}

} // namespace detail
} // namespace rc
