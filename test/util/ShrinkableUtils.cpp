#include "ShrinkableUtils.h"

#include "rapidcheck/gen/Container.h"
#include "rapidcheck/seq/Create.h"

namespace rc {
namespace test {

Gen<std::vector<std::size_t>> genPath() {
  return gen::container<std::vector<std::size_t>>(
      Gen<std::size_t>([](const Random &random, int size) {
        std::size_t max = (size * 100) / kNominalSize;
        return shrinkable::shrinkRecur(
            static_cast<std::size_t>(Random(random).next() % max),
            [](std::size_t x) { return shrink::towards<std::size_t>(x, 0); });
      }));
}

Seq<int> countdownSeq(int x) {
  return seq::range(x - 1, -1);
}

Shrinkable<int> countdownShrinkable(int n) {
  return shrinkable::shrinkRecur(n, &countdownSeq);
}

} // namespace test
} // namespace rc
