#pragma once

#include <cstddef>

#include "rapidcheck/detail/RandomEngine.h"
#include "rapidcheck/detail/Variant.h"
#include "rapidcheck/Generator.h"

namespace rc {
namespace detail {

//! Describes a particular test case.
struct TestCase
{
    //! The test case index.
    int index = 0;
    //! The used size.
    size_t size = 0;
    //! The used seed.
    RandomEngine::Atom seed = 0;
};

//! Describes the result of a test case.
class CaseResult
{
public:
    //! The type of the result.
    enum class Type {
        Success, //!< The test case succeeded.
        Failure, //!< The test case failed.
        Discard  //!< The preconditions for the test case were not met.
    };

    //! Creates a new `Result` of the given type and with the given description.
    explicit CaseResult(Type type, std::string description = "");

    //! Returns the type.
    Type type() const;

    //! Returns the description.
    std::string description() const;

    //! Two results are equal if they have the same type and description and
    //! if both have or both doesn't have an exception.
    bool operator==(const CaseResult &rhs) const;

    //! Opposite of `operator==`
    bool operator!=(const CaseResult &rhs) const;

private:
    Type m_type;
    std::string m_description;
};

struct SuccessResult
{
    //! The number of tests run.
    int numTests;
};

struct FailureResult
{
    //! The failing test case.
    TestCase failingCase;
    //! A description of the failure.
    std::string description;
    //! The number of shrinks performed.
    int numShrinks;
    //! The counterexample.
    std::vector<gen::ValueDescription> counterExample;
};

//! Describes the circumstances around the result of a test.
typedef Variant<SuccessResult, FailureResult> TestResult;

//! Returns a short message describing the given test results.
std::string resultMessage(const TestResult &result);

void show(CaseResult::Type type, std::ostream &os);
void show(const CaseResult &result, std::ostream &os);

} // namespace detail
} // namespace rc
