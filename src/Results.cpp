#include "rapidcheck/detail/Results.h"

namespace rc {
namespace detail {

CaseResult::CaseResult(Type type, std::string description)
    : m_type(type)
    , m_description(description) {}

CaseResult::Type CaseResult::type() const { return m_type; }
std::string CaseResult::description() const { return m_description; }

bool CaseResult::operator==(const CaseResult &rhs) const
{
    return
        (m_type == rhs.m_type) &&
        (m_description == rhs.m_description);
}

bool CaseResult::operator!=(const CaseResult &rhs) const
{ return !(*this == rhs); }

std::string resultMessage(const TestResult &result)
{
    SuccessResult success;
    FailureResult failure;

    if (result.match(success)) {
        return "OK, passed " + std::to_string(success.numTests) + " tests";
    } else if (result.match(failure)) {
        std::string msg;
        msg += "Falsifiable after " + std::to_string(failure.failingCase.index);
        msg += " tests";
        if (failure.numShrinks > 0) {
            msg += " and " + std::to_string(failure.numShrinks);
            msg += (failure.numShrinks == 1) ? " shrink" : " shrinks";
        }

        return msg;
    }

    return std::string();
}

void show(CaseResult::Type type, std::ostream &os)
{
    switch (type) {
    case CaseResult::Type::Success:
        os << "Success";
        break;

    case CaseResult::Type::Failure:
        os << "Failure";
        break;

    case CaseResult::Type::Discard:
        os << "Discard";
        break;
    }
}

void show(const CaseResult &result, std::ostream &os)
{
    show(result.type(), os);
    os << ": " << result.description();
}

} // namespace detail
} // namespace rc
