#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "util/Generators.h"

using namespace rc;

TEST_CASE("SeqIterator") {
  SECTION("operator==") {
    SECTION("default-constructed equals default constructed") {
      REQUIRE(seq::SeqIterator<int>() == seq::SeqIterator<int>());
    }

    SECTION("default-constructed equals iterator for empty Seq") {
      REQUIRE(seq::SeqIterator<int>() == begin(Seq<int>()));
    }

    prop("other iterators are never equal",
         [] {
           const auto seq =
               *gen::suchThat<Seq<int>>([](Seq<int> s) { return !!s.next(); });
           RC_ASSERT(begin(seq) != begin(seq));
         });
  }

  prop("works with container initialization",
       [](const std::vector<int> &elements) {
         const auto seq = seq::fromContainer(elements);
         std::vector<int> newElements(begin(seq), end(seq));
         RC_ASSERT(newElements == elements);
       });

  prop("works with range based for-loops",
       [](const std::vector<int> &elements) {
         const auto seq = seq::fromContainer(elements);
         std::vector<int> actual;
         actual.reserve(elements.size());
         for (auto x : seq) {
           actual.push_back(x);
         }

         RC_ASSERT(actual == elements);
       });
}
