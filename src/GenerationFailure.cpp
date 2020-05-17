#include "rapidcheck/GenerationFailure.h"


#include <string>
#include <utility>

namespace rc {

GenerationFailure::GenerationFailure(std::string msg)
    : std::runtime_error(std::move(msg)) {}

} // namespace rc
