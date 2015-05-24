#include "rapidcheck/gen/detail/ScaleInteger.h"

#include <algorithm>

#include "rapidcheck/Gen.h"

namespace rc {
namespace gen {
namespace detail {
namespace {

uint64_t mulDiv(uint64_t a, uint64_t b, uint64_t c) {
  const uint64_t ah = a >> 32;
  const uint64_t al = a & 0xFFFFFFFFULL;
  const uint64_t bh = b >> 32;
  const uint64_t bl = b & 0xFFFFFFFFULL;

  const uint64_t x1 = ((ah * bl) / c) << 32;
  const uint64_t x2 = ((al * bh) / c) << 32;
  const uint64_t x3 = (al * bl) / c;
  return x1 + x2 + x3;
}

} // namespace

uint64_t scaleInteger(uint64_t x, int size) {
  const auto clampedSize = std::min(kNominalSize, size);
  return mulDiv(x, clampedSize, kNominalSize);
}

} // namespace detail
} // namespace gen
} // namespace rc
