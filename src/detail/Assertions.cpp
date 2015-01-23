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
        throw CaseResult(
            type, file + ":" + std::to_string(line) + ": " + description);
    }
}

} // namespace detail
} // namespace rc
