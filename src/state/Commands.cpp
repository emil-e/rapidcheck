#include "rapidcheck/state/Commands.h"

namespace rc {
namespace state {
namespace detail {

void rethrowOnException(std::exception_ptr e) {
  if (e != nullptr) {
    std::rethrow_exception(e);
  };
}

} // namespace detail
} // namespace state
} // namespace rc
