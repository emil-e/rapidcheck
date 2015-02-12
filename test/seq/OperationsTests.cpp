#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Operations.h"
#include "rapidcheck/seq/Create.h"

using namespace rc;

TEST_CASE("seq::length") {
    prop("returns the number of elements in the sequence",
         [] (const std::vector<int> &elements) {
             auto seq = seq::fromContainer(elements);
             RC_ASSERT(seq::length(seq) == elements.size());
         });
}
