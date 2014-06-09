#include "rapidcheck/detail/Results.h"

namespace rc {
namespace detail {

std::ostream &operator<<(std::ostream &os, Result result)
{
    switch (result) {
    case Result::Success:
        os << "Success";
        break;

    case Result::Failure:
        os << "Failure";
        break;
    }

    return os;
}

} // namespace detail
} // namespace rc
