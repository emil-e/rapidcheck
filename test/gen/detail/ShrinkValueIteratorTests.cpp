#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/Shrinkable.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/gen/detail/ShrinkValueIterator.h"

#include "util/Predictable.h"

using namespace rc;
using namespace rc::gen::detail;

TEST_CASE("ShrinkValueIterator") {
    prop(
        "returns the value of shrinkable referenced by the underlying"
        " iterator",
        [](const std::vector<int> &elements) {
            std::vector<Shrinkable<int>> shrinkables;
            shrinkables.reserve(elements.size());
            std::transform(
                begin(elements), end(elements), std::back_inserter(shrinkables),
                &shrinkable::just<const int &>);
            std::vector<int> result(
                makeShrinkValueIterator(begin(shrinkables)),
                makeShrinkValueIterator(end(shrinkables)));
            RC_ASSERT(result == elements);
        });

    prop(
        "works with non-copyable elements",
        [](const std::vector<NonCopyable> &elements) {
            std::vector<Shrinkable<NonCopyable>> shrinkables;
            shrinkables.reserve(elements.size());
            std::transform(
                begin(elements), end(elements), std::back_inserter(shrinkables),
                [](const NonCopyable &x) {
                    const auto value = x.value;
                    const auto extra = x.extra;
                    return shrinkable::lambda([=] {
                        NonCopyable nc;
                        nc.value = value;
                        nc.extra = extra;
                        return nc;
                    });
                });
            std::vector<NonCopyable> result(
                makeShrinkValueIterator(begin(shrinkables)),
                makeShrinkValueIterator(end(shrinkables)));
            RC_ASSERT(result == elements);
        });
}
