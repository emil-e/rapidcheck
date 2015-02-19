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
         [](const std::vector<std::string> &elements) {
             auto seq = seq::fromContainer(elements);
             auto it = begin(elements);
             seq::forEach(seq, [&](const std::string &x) {
                 RC_ASSERT(x == *it++);
             });
             RC_ASSERT(it == end(elements));
         });
}

TEST_CASE("seq::last") {
    SECTION("returns Nothing for empty Seq") {
        REQUIRE(!seq::last(Seq<int>()));
    }

    prop("returns the last element of the Seq",
         []{
             auto elements = *gen::suchThat<std::vector<int>>(
                 [](const std::vector<int> &x) { return !x.empty(); });
             RC_ASSERT(*seq::last(seq::fromContainer(elements)) ==
                       elements.back());
         });
}

TEST_CASE("seq::contains") {
    prop("returns false for elements not in Seq",
         [](const std::vector<int> &elements) {
             int value = *gen::suchThat<int>(
                 [&](int x) {
                     return std::find(begin(elements),
                                      end(elements),
                                      x) == end(elements);
                 });
             RC_ASSERT(!seq::contains(seq::fromContainer(elements), value));
         });

    prop("returns true for elements in Seq",
         []{
             auto elements = *gen::suchThat<std::vector<int>>(
                 [](const std::vector<int> &x) { return !x.empty(); });
             int value = *gen::elementOf(elements);
             RC_ASSERT(seq::contains(seq::fromContainer(elements), value));
         });
}
