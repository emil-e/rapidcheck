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
             std::size_t n = *gen::ranged<std::size_t>(
                 0, (elements.size() + 1) * 2);
             std::size_t start = std::min(elements.size(), n);
             std::vector<int> rest(elements.begin() + start, elements.end());
             RC_ASSERT(seq::drop(n, seq::fromContainer(elements)) ==
                       seq::fromContainer(rest));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements) {
             std::size_t n = *gen::ranged<std::size_t>(
                 0, (elements.size() + 1) * 2);
             assertEqualCopies(seq::drop(n, seq::fromContainer(elements)));
         });

    prop("does not copy items",
         [] (std::vector<CopyGuard> elements) {
             std::size_t n = *gen::ranged<std::size_t>(
                 0, (elements.size() + 1) * 2);
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
             std::size_t n = *gen::ranged<std::size_t>(
                 0, (elements.size() + 1) * 2);
             std::size_t start = std::min(elements.size(), n);
             std::vector<int> head(elements.begin(), elements.begin() + start);
             RC_ASSERT(seq::take(n, seq::fromContainer(elements)) ==
                       seq::fromContainer(head));
         });

    prop("copies are equal",
         [] (const std::vector<int> &elements) {
             std::size_t n = *gen::ranged<std::size_t>(
                 0, (elements.size() + 1) * 2);
             std::size_t start = std::min(elements.size(), n);
             assertEqualCopies(seq::take(n, seq::fromContainer(elements)));
         });

    prop("does not copy items",
         [] (std::vector<CopyGuard> elements) {
             std::size_t n = *gen::ranged<std::size_t>(
                 0, (elements.size() + 1) * 2);
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
                                          [=](int x) { return !pred(x); });
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
                                          [=](int x) { return !pred(x); });
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
    prop("maps elements using mapping callable",
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

    prop("copies are equal",
         [] (const std::vector<int> &elements, int x)
         {
             const auto mapper = [=](int a) { return a * x; };
             auto mapSeq = seq::map(mapper, seq::fromContainer(elements));
             assertEqualCopies(mapSeq);
         });

    prop("does not copy elements",
         [] (std::vector<CopyGuard> elements) {
             const auto mapper = [](CopyGuard &&x) { return std::move(x); };
             auto mapSeq = seq::map(
                 mapper, seq::fromContainer(std::move(elements)));
             while (mapSeq.next());
         });
}

TEST_CASE("seq::zipWith") {
    prop("works with no sequences",
         [] (int x) {
             std::size_t n = *gen::ranged<std::size_t>(0, 1000);
             const auto mapper = [=]{ return x; };
             auto zipSeq = seq::take(n, seq::zipWith(mapper));
             for (std::size_t i = 0; i < n; i++)
                 RC_ASSERT(*zipSeq.next() == x);
             RC_ASSERT(!zipSeq.next());
         });

    prop("works with one sequence",
         [] (const std::vector<int> &elements, int x)
         {
             const auto mapper = [=](int a) { return a * x; };
             std::vector<int> expectedElements;
             for (const auto e : elements)
                 expectedElements.push_back(mapper(e));
             auto zipSeq = seq::zipWith(mapper, seq::fromContainer(elements));
             RC_ASSERT(zipSeq ==
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
             auto zipSeq = seq::zipWith(mapper,
                                        seq::fromContainer(e1),
                                        seq::fromContainer(e2));
             RC_ASSERT(zipSeq ==
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
             auto zipSeq = seq::zipWith(mapper,
                                        seq::fromContainer(e1),
                                        seq::fromContainer(e2),
                                        seq::fromContainer(e3));
             RC_ASSERT(zipSeq ==
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
             auto zipSeq = seq::zipWith(mapper,
                                        seq::fromContainer(e1),
                                        seq::fromContainer(e2),
                                        seq::fromContainer(e3));
             assertEqualCopies(zipSeq);
         });

    prop("does not copy elements",
         [] (std::vector<CopyGuard> e1, std::vector<CopyGuard> e2) {
             const auto mapper = [](CopyGuard &&a, CopyGuard &&b) {
                 return std::make_pair(std::move(a), std::move(b));
             };
             auto zipSeq = seq::zipWith(mapper,
                                        seq::fromContainer(std::move(e1)),
                                        seq::fromContainer(std::move(e2)));
             while (zipSeq.next());
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

TEST_CASE("seq::join") {
    auto subgen = gen::scale(0.25, gen::arbitrary<std::vector<int>>());
    auto gen = gen::collection<std::vector<std::vector<int>>>(subgen);

    prop("returns a seq with the elements of all seqs joined together",
         [&] {
             auto vectors = *gen;
             auto seqs = seq::map([](std::vector<int> &&vec) {
                 return seq::fromContainer(std::move(vec));
             }, seq::fromContainer(vectors));

             std::vector<int> expectedElements;
             for (const auto &vec : vectors) {
                 expectedElements.insert(expectedElements.end(),
                                         begin(vec), end(vec));
             }

             RC_ASSERT(seq::join(seqs) == seq::fromContainer(expectedElements));
         });

    prop("copies are equal",
         [&] {
             auto vectors = *gen;
             auto seqs = seq::map([](std::vector<int> &&vec) {
                 return seq::fromContainer(std::move(vec));
             }, seq::fromContainer(vectors));
             assertEqualCopies(seq::join(seqs));
         });

    prop("does not copy elements",
         [&] {
             auto subguardgen =
                 gen::scale(0.25, gen::arbitrary<std::vector<CopyGuard>>());
             auto guardgen =
                 gen::collection<std::vector<std::vector<CopyGuard>>>(
                     subguardgen);
             auto vectors = *guardgen;
             auto seqs = seq::map([](std::vector<CopyGuard> &&vec) {
                 return seq::fromContainer(std::move(vec));
             }, seq::fromContainer(std::move(vectors)));
             auto seq = seq::join(std::move(seqs));
             while (seq.next());
         });
}

TEST_CASE("seq::concat") {
    prop("joins the given sequences together",
         [] (const std::vector<int> &a,
             const std::vector<int> &b,
             const std::vector<int> &c)
         {
             std::vector<int> expectedElements;
             expectedElements.insert(end(expectedElements), begin(a), end(a));
             expectedElements.insert(end(expectedElements), begin(b), end(b));
             expectedElements.insert(end(expectedElements), begin(c), end(c));

             RC_ASSERT(seq::concat(seq::fromContainer(a),
                                   seq::fromContainer(b),
                                   seq::fromContainer(c)) ==
                       seq::fromContainer(expectedElements));
         });

    prop("copies are equal",
         [] (const std::vector<int> &a,
             const std::vector<int> &b,
             const std::vector<int> &c)
         {
             assertEqualCopies(seq::concat(seq::fromContainer(a),
                                           seq::fromContainer(b),
                                           seq::fromContainer(c)));
         });

    prop("does not copy elements",
         [] (std::vector<CopyGuard> a,
             std::vector<CopyGuard> b,
             std::vector<CopyGuard> c)
         {
             auto seq = seq::concat(seq::fromContainer(std::move(a)),
                                    seq::fromContainer(std::move(b)),
                                    seq::fromContainer(std::move(c)));
             while (seq.next());
         });
}

TEST_CASE("seq::mapcat") {
    prop("equivalent to seq::join(seq::map(...))",
         [](std::vector<int> elements) {
             const auto seq = seq::fromContainer(std::move(elements));
             const auto mapper = [](int a) {
                 return seq::just(a, a + 1, a + 2);
             };

             RC_ASSERT(seq::mapcat(mapper, seq) ==
                       seq::join(seq::map(mapper, seq)));
         });

    prop("copies are equal",
         [](std::vector<int> elements) {
             const auto seq = seq::fromContainer(std::move(elements));
             const auto mapper = [](int a) {
                 return seq::just(a, a + 1, a + 2);
             };

             assertEqualCopies(seq::mapcat(mapper, seq));
         });

    prop("does not copy elements",
         [](std::vector<CopyGuard> elements) {
             auto seq = seq::fromContainer(std::move(elements));
             const auto mapper = [](CopyGuard &&a) {
                 return seq::just(std::move(a), CopyGuard(1337));
             };
             auto mapSeq = seq::mapcat(mapper, std::move(seq));
             while (mapSeq.next());
         });
}

TEST_CASE("seq::mapMaybe") {
    prop("removes elements for which mapper evaluates to Nothing",
         [](const std::vector<int> &elements) {
             const auto seq = seq::fromContainer(elements);
             const auto pred = [](int x) { return (x % 2) == 0; };
             const auto maybeSeq = seq::mapMaybe(
                 [=](int x) -> Maybe<int> {
                     return pred(x) ? Maybe<int>(x) : Nothing;
                 }, seq);
             RC_ASSERT(maybeSeq == seq::filter(pred, seq));
         });

    prop("for elements that are not Nothing, unwraps the contents",
         [](const std::vector<int> &elements) {
             const auto seq = seq::fromContainer(elements);
             const auto mapper = [](int x) { return x * x; };
             const auto maybeSeq = seq::mapMaybe(
                 [=](int x) -> Maybe<int> { return mapper(x); }, seq);
             RC_ASSERT(maybeSeq == seq::map(mapper, seq));
         });

    prop("copies are equal",
         [](const std::vector<int> &elements) {
             const auto seq = seq::fromContainer(elements);
             assertEqualCopies(
                 seq::mapMaybe(
                     [=](int x) -> Maybe<int> { return x * x; },
                     seq));
         });

    prop("does not copy elements",
         [](std::vector<CopyGuard> elements) {
             auto seq = seq::fromContainer(std::move(elements));
             auto maybeSeq = seq::mapMaybe(
                 [=](CopyGuard x) -> Maybe<CopyGuard> { return std::move(x); },
                 std::move(seq));
             while (maybeSeq.next());
         });
}

TEST_CASE("seq::cycle") {
    prop("returns an infinite cycle of the given Seq",
         [] {
             auto elements = *gen::suchThat<std::vector<int>>(
                 [](const std::vector<int> &x) { return !x.empty(); });
             auto seq = seq::cycle(seq::fromContainer(elements));
             auto it = begin(elements);
             for (int i = 0; i < 2000; i++) {
                 RC_ASSERT(*seq.next() == *it++);
                 if (it == end(elements))
                     it = begin(elements);
             }
         });

    prop("does not copy Seq on construction",
         [] {
             auto elements = *gen::suchThat<std::vector<CopyGuard>>(
                 [](const std::vector<CopyGuard> &x) { return !x.empty(); });
             auto seq = seq::cycle(seq::fromContainer(std::move(elements)));
         });

    prop("copies are equal",
         [] {
             auto elements = *gen::suchThat<std::vector<int>>(
                 [](const std::vector<int> &x) { return !x.empty(); });
             auto seq = seq::cycle(seq::fromContainer(elements));
             assertEqualCopies(seq::take(1000, std::move(seq)));
         });
}

TEST_CASE("seq::cast") {
    prop("casting to a larger type and then back yields Seq equal to original",
         [](const std::vector<uint8_t> &elements) {
             const auto seq = seq::fromContainer(elements);
             RC_ASSERT(seq::cast<uint8_t>(seq::cast<int>(seq)) == seq);
         });

    prop("copies are equal",
         [](const std::vector<uint8_t> &elements) {
             assertEqualCopies(
                 seq::cast<int>(seq::fromContainer(elements)));
         });

    prop("does not copy elements",
         [](std::vector<CopyGuard> elements) {
             auto seq = seq::cast<CopyGuard>(
                 seq::fromContainer(std::move(elements)));
             while (seq.next());
         });
}
