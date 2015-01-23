#include "rapidcheck/gen/Generator.h"

namespace rc {
namespace gen {

GenerationFailure::GenerationFailure(std::string msg)
    : std::runtime_error(std::move(msg)) {}

int currentSize()
{
    return *detail::ImplicitParam<detail::param::Size>();
}

} // namespace gen
} // namespace rc
