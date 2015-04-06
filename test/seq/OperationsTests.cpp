#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Operations.h"
#include "rapidcheck/seq/Create.h"

using namespace rc;

TEST_CASE("seq::length") {
    newprop(
        "returns the number of elements in the sequence",
        [](const std::vector<int> &elements) {
            auto seq = seq::fromContainer(elements);
            RC_ASSERT(seq::length(seq) == elements.size());
        });
}

TEST_CASE("seq::forEach") {
    newprop(
        "calls the callable once for each element",
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

    newprop(
        "returns the last element of the Seq",
        []{
            auto elements = *newgen::suchThat<std::vector<int>>(
                [](const std::vector<int> &x) { return !x.empty(); });
            RC_ASSERT(*seq::last(seq::fromContainer(elements)) ==
                      elements.back());
        });
}

TEST_CASE("seq::contains") {
    newprop(
        "returns false for elements not in Seq",
        [](const std::vector<int> &elements) {
            int value = *newgen::suchThat<int>(
                [&](int x) {
                    return std::find(begin(elements),
                                     end(elements),
                                     x) == end(elements);
                });
            RC_ASSERT(!seq::contains(seq::fromContainer(elements), value));
        });

    newprop(
        "returns true for elements in Seq",
        []{
            auto elements = *newgen::suchThat<std::vector<int>>(
                [](const std::vector<int> &x) { return !x.empty(); });
            int value = *newgen::elementOf(elements);
            RC_ASSERT(seq::contains(seq::fromContainer(elements), value));
        });
}

TEST_CASE("seq::all") {
    newprop(
        "returns true if all elements match predicate",
        [] (int value) {
            int n = *newgen::inRange<std::size_t>(0, 200);
            auto seq = seq::take(n, seq::repeat(value));
            RC_ASSERT(seq::all(seq, [=](int x) { return x == value; }));
        });

    newprop(
        "returns false if one element does not match predicate",
        [] (int value) {
            int other = *newgen::distinctFrom(value);
            int n1 = *newgen::inRange<std::size_t>(0, 100);
            int n2 = *newgen::inRange<std::size_t>(0, 100);
            auto seq = seq::concat(seq::take(n1, seq::repeat(value)),
                                   seq::just(other),
                                   seq::take(n2, seq::repeat(value)));
            RC_ASSERT(!seq::all(seq, [=](int x) { return x == value; }));
        });
}

TEST_CASE("seq::any") {
    newprop(
        "seq::any(..., pred) != seq::all(..., !pred)",
        [] {
            auto someNumber = newgen::inRange<std::size_t>(0, 200);
            int x = *someNumber;
            auto elements = *newgen::container<std::vector<int>>(someNumber);
            auto seq = seq::fromContainer(elements);
            const auto pred = [=](int y) { return x == y; };
            const auto npred = [=](int y) { return x != y; };
            RC_ASSERT(seq::any(seq, pred) != seq::all(seq, npred));
        });
}

TEST_CASE("seq::at") {
    newprop(
        "returns the element at the given position if index < size",
        [] {
            const auto elements = *newgen::suchThat<std::vector<int>>(
                [](const std::vector<int> &x) { return !x.empty(); });
            std::size_t i = *newgen::inRange<std::size_t>(0, elements.size());
            auto x = seq::at(seq::fromContainer(elements), i);
            RC_ASSERT(x);
            RC_ASSERT(*x == elements[i]);
        });

    newprop(
        "returns Nothing for indexes past end",
        [](int x) {
            std::size_t n = *newgen::inRange<std::size_t>(0, 100);
            std::size_t i = *newgen::inRange<std::size_t>(n, 200);
            RC_ASSERT(!seq::at(seq::take(n, seq::repeat(x)), i));
        });
}
