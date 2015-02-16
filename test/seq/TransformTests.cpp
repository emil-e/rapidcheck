#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"

#include "util/CopyGuard.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("seq::drop") {
    prop("drops the first N elements from the given seq",
         [] (const std::vector<int> &elements) {
             std::size_t n = *gen::ranged<std::size_t>(0, elements.size() * 2);
             std::size_t start = std::min(elements.size(), n);
             std::vector<int> rest(elements.begin() + start, elements.end());
             RC_ASSERT(seq::drop(n, seq::fromContainer(elements)) ==
                       seq::fromContainer(rest));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements) {
             std::size_t n = *gen::ranged<std::size_t>(0, elements.size() * 2);
             auto seq = seq::drop(n, seq::fromContainer(elements));
             std::size_t nexts =
                 *gen::ranged<std::size_t>(0, elements.size() * 2);
             while (nexts--)
                 seq.next();
             const auto copy = seq;
             RC_ASSERT(seq == copy);
         });

    prop("does not copy items",
         [] (std::vector<CopyGuard> elements) {
             std::size_t n = *gen::ranged<std::size_t>(0, elements.size() * 2);
             auto seq = seq::drop(n, seq::fromContainer(std::move(elements)));
             while (seq.next());
         });

    SECTION("sanity check") {
        REQUIRE(seq::drop(2, seq::just(1, 2, 3)) == seq::just(3));
    }
}

TEST_CASE("seq::take") {
    prop("takes the first N elements from the given seq",
         [] (const std::vector<int> &elements) {
             std::size_t n = *gen::ranged<std::size_t>(0, elements.size() * 2);
             std::size_t start = std::min(elements.size(), n);
             std::vector<int> head(elements.begin(), elements.begin() + start);
             RC_ASSERT(seq::take(n, seq::fromContainer(elements)) ==
                       seq::fromContainer(head));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements) {
             std::size_t n = *gen::ranged<std::size_t>(0, elements.size() * 2);
             std::size_t start = std::min(elements.size(), n);
             auto seq = seq::take(n, seq::fromContainer(elements));
             std::size_t nexts =
                 *gen::ranged<std::size_t>(0, elements.size() * 2);
             while (nexts--)
                 seq.next();
             const auto copy = seq;
             RC_ASSERT(seq == copy);
         });

    prop("does not copy items",
         [] (std::vector<CopyGuard> elements) {
             std::size_t n = *gen::ranged<std::size_t>(0, elements.size() * 2);
             std::size_t start = std::min(elements.size(), n);
             auto seq = seq::take(n, seq::fromContainer(std::move(elements)));
             while (seq.next());
         });

    SECTION("sanity check") {
        REQUIRE(seq::take(2, seq::just(1, 2, 3)) == seq::just(1, 2));
    }
}

TEST_CASE("seq::dropWhile") {
    prop("drops all elements before the first element matching the predicate",
         [] (const std::vector<int> &elements, int limit) {
             const auto pred = [=](int x) { return x < limit; };
             const auto it = std::find_if(begin(elements), end(elements),
                                          [&](int x) { return !pred(x); });
             RC_ASSERT(seq::dropWhile(pred, seq::fromContainer(elements)) ==
                       seq::fromIteratorRange(it, end(elements)));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements, int limit) {
             const auto pred = [=](int x) { return x < limit; };
             auto seq = seq::dropWhile(pred, seq::fromContainer(elements));
             std::size_t nexts =
                 *gen::ranged<std::size_t>(0, elements.size() * 2);
             while (nexts--)
                 seq.next();
             const auto copy = seq;
             RC_ASSERT(seq == copy);
         });

    prop("does not copy items",
         [] (std::vector<CopyGuard> elements, int limit) {
             const auto pred = [=](const CopyGuard &x) { return x < limit; };
             auto seq = seq::dropWhile(pred, seq::fromContainer(std::move(elements)));
             while (seq.next());
         });

    SECTION("sanity check") {
        auto seq = seq::dropWhile(
            [](int x) { return x < 5; },
            seq::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
        auto expected = seq::just(5, 6, 7, 8, 9, 10);
        REQUIRE(seq == expected);
    }
}

TEST_CASE("seq::takeWhile") {
    prop("takes all elements before the first element matching the predicate",
         [] (const std::vector<int> &elements, int limit) {
             const auto pred = [=](int x) { return x < limit; };
             const auto it = std::find_if(begin(elements), end(elements),
                                          [&](int x) { return !pred(x); });
             RC_ASSERT(seq::takeWhile(pred, seq::fromContainer(elements)) ==
                       seq::fromIteratorRange(begin(elements), it));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements, int limit) {
             const auto pred = [=](int x) { return x < limit; };
             auto seq = seq::takeWhile(pred, seq::fromContainer(elements));
             std::size_t nexts =
                 *gen::ranged<std::size_t>(0, elements.size() * 2);
             while (nexts--)
                 seq.next();
             const auto copy = seq;
             RC_ASSERT(seq == copy);
         });

    prop("does not copy items",
         [] (std::vector<CopyGuard> elements, int limit) {
             const auto pred = [=](const CopyGuard &x) { return x < limit; };
             auto seq = seq::takeWhile(pred, seq::fromContainer(std::move(elements)));
             while (seq.next());
         });

    SECTION("sanity check") {
        auto seq = seq::takeWhile(
            [](int x) { return x < 5; },
            seq::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
        auto expected = seq::just(0, 1, 2, 3, 4);
        REQUIRE(seq == expected);
    }
}
