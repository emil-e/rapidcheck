#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"
#include "rapidcheck/seq/Operations.h"

#include "util/SeqUtils.h"
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
             assertEqualCopies(seq::drop(n, seq::fromContainer(elements)));
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
             assertEqualCopies(seq::take(n, seq::fromContainer(elements)));
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
             assertEqualCopies(seq::dropWhile(pred, seq::fromContainer(elements)));
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
             assertEqualCopies(seq::takeWhile(pred, seq::fromContainer(elements)));
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

TEST_CASE("seq::map") {
    prop("works with no sequences",
         [] (int x) {
             std::size_t n = *gen::ranged<std::size_t>(0, 1000);
             const auto mapper = [=]{ return x; };
             auto mapSeq = seq::take(n, seq::map(mapper));
             for (std::size_t i = 0; i < n; i++)
                 RC_ASSERT(*mapSeq.next() == x);
             RC_ASSERT(!mapSeq.next());
         });

    prop("works with one sequence",
         [] (const std::vector<int> &elements, int x)
         {
             const auto mapper = [=](int a) { return a * x; };
             std::vector<int> expectedElements;
             for (const auto e : elements)
                 expectedElements.push_back(mapper(e));
             auto mapSeq = seq::map(mapper, seq::fromContainer(elements));
             RC_ASSERT(mapSeq ==
                       seq::fromContainer(std::move(expectedElements)));
         });

    prop("works with two sequences",
         [] (const std::vector<int> &e1,
             const std::vector<std::string> &e2)
         {
             const auto mapper = [](int a, std::string b) {
                 return std::to_string(a) + b;
             };
             std::size_t size = std::min(e1.size(), e2.size());
             std::vector<std::string> expectedElements;
             for (std::size_t i = 0; i < size; i++)
                 expectedElements.push_back(mapper(e1[i], e2[i]));
             auto mapSeq = seq::map(mapper,
                                    seq::fromContainer(e1),
                                    seq::fromContainer(e2));
             RC_ASSERT(mapSeq ==
                       seq::fromContainer(std::move(expectedElements)));
         });

    prop("works with three sequences",
         [] (const std::vector<int> &e1,
             const std::vector<std::string> &e2,
             const std::vector<double> &e3)
         {
             const auto mapper = [](int a, std::string b, double c) {
                 return std::to_string(a) + b + std::to_string(c);
             };
             std::size_t size = std::min({e1.size(), e2.size(), e3.size()});
             std::vector<std::string> expectedElements;
             for (std::size_t i = 0; i < size; i++)
                 expectedElements.push_back(mapper(e1[i], e2[i], e3[i]));
             auto mapSeq = seq::map(mapper,
                                    seq::fromContainer(e1),
                                    seq::fromContainer(e2),
                                    seq::fromContainer(e3));
             RC_ASSERT(mapSeq ==
                       seq::fromContainer(std::move(expectedElements)));
         });

    prop("copies are equal",
         [] (const std::vector<int> &e1,
             const std::vector<std::string> &e2,
             const std::vector<double> &e3)
         {
             const auto mapper = [](int a, std::string b, double c) {
                 return std::to_string(a) + b + std::to_string(c);
             };
             auto mapSeq = seq::map(mapper,
                                    seq::fromContainer(e1),
                                    seq::fromContainer(e2),
                                    seq::fromContainer(e3));
             assertEqualCopies(mapSeq);
         });

    prop("does not copy elements",
         [] (std::vector<CopyGuard> e1, std::vector<CopyGuard> e2) {
             const auto mapper = [](CopyGuard &&a, CopyGuard &&b) {
                 return std::make_pair(std::move(a), std::move(b));
             };
             auto mapSeq = seq::map(mapper,
                                    seq::fromContainer(std::move(e1)),
                                    seq::fromContainer(std::move(e2)));
             while (mapSeq.next());
         });
}

TEST_CASE("seq::filter") {
    prop("returns a seq with only the elements not matching the predicate"
         " removed",
         [] (const std::vector<int> &elements, int limit) {
             const auto pred = [=](int x) { return x < limit; };
             std::vector<int> expectedElements;
             std::copy_if(begin(elements), end(elements),
                          std::back_inserter(expectedElements),
                          pred);

             RC_ASSERT(seq::filter(pred, seq::fromContainer(elements)) ==
                       seq::fromContainer(std::move(expectedElements)));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements, int limit) {
             const auto pred = [=](int x) { return x < limit; };
             std::vector<int> expectedElements;
             std::copy_if(begin(elements), end(elements),
                          std::back_inserter(expectedElements),
                          pred);
             assertEqualCopies(seq::filter(pred, seq::fromContainer(elements)));
         });

    prop("does not copy elements",
         [] (std::vector<CopyGuard> elements, int limit) {
             const auto pred = [=](const CopyGuard &x) { return x.value < limit; };
             auto seq = seq::filter(pred, seq::fromContainer(std::move(elements)));
             while (seq.next());
         });
}
