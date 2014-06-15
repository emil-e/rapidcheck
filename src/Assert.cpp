#include "rapidcheck/Assert.h"

#include "rapidcheck/detail/Results.h"

namespace rc {
namespace detail {

void assertThat(std::string description, bool result)
{
    if (!result)
        throw CaseResult(CaseResult::Type::Failure);
}

} // namespace detail
} // namespace rc
