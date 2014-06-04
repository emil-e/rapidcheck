#include "rapidcheck/Show.h"

namespace rc {
namespace detail {
namespace ansi {

std::string cursorUp(int n)
{
    return "\e[" + std::to_string(n) + "A";
}

std::string cursorDown(int n)
{
    return "\e[" + std::to_string(n) + "B";
}

} // namespace ansi
} // namespace detail
} // namespace rc
