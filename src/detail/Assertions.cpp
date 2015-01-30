#include "rapidcheck/Assertions.h"

namespace rc {
namespace detail {

void throwResultIf(CaseResult::Type type,
                   bool condition,
                   std::string description,
                   std::string file,
                   int line)
{
    if (condition) {
        auto desc = file + ":" + std::to_string(line) + ": " + description;
        throw CaseResult {
            .type = type,
            .description = desc
        };
    }
}

} // namespace detail
} // namespace rc
