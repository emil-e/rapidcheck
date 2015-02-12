#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Operations.h"
#include "rapidcheck/seq/Create.h"

using namespace rc;

TEST_CASE("seq::length") {
    prop("returns the number of elements in the sequence",
         [](const std::vector<int> &elements) {
             auto seq = seq::fromContainer(elements);
             RC_ASSERT(seq::length(seq) == elements.size());
         });
}

TEST_CASE("seq::forEach") {
    prop("calls the callable once for each element",
         [](const std::vector<int> &elements) {
             auto seq = seq::fromContainer(elements);
             auto it = begin(elements);
             seq::forEach(seq, [&](int x) {
                 RC_ASSERT(x == *it++);
             });
             RC_ASSERT(it == end(elements));
         });
}
