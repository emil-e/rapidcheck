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
    newprop(
        "drops the first N elements from the given seq",
        [] (const std::vector<int> &elements) {
            std::size_t n = *newgen::inRange<std::size_t>(
                0, (elements.size() + 1) * 2);
            std::size_t start = std::min(elements.size(), n);
            std::vector<int> rest(elements.begin() + start, elements.end());
            RC_ASSERT(seq::drop(n, seq::fromContainer(elements)) ==
                      seq::fromContainer(rest));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &elements) {
            std::size_t n = *newgen::inRange<std::size_t>(
                0, (elements.size() + 1) * 2);
            assertEqualCopies(seq::drop(n, seq::fromContainer(elements)));
        });

    newprop(
        "does not copy items",
        [] (std::vector<CopyGuard> elements) {
            std::size_t n = *newgen::inRange<std::size_t>(
                0, (elements.size() + 1) * 2);
            auto seq = seq::drop(n, seq::fromContainer(std::move(elements)));
            while (seq.next());
        });

    SECTION("sanity check") {
        REQUIRE(seq::drop(2, seq::just(1, 2, 3)) == seq::just(3));
    }
}

TEST_CASE("seq::take") {
    newprop(
        "takes the first N elements from the given seq",
        [] (const std::vector<int> &elements) {
            std::size_t n = *newgen::inRange<std::size_t>(
                0, (elements.size() + 1) * 2);
            std::size_t start = std::min(elements.size(), n);
            std::vector<int> head(elements.begin(), elements.begin() + start);
            RC_ASSERT(seq::take(n, seq::fromContainer(elements)) ==
                      seq::fromContainer(head));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &elements) {
            std::size_t n = *newgen::inRange<std::size_t>(
                0, (elements.size() + 1) * 2);
            std::size_t start = std::min(elements.size(), n);
            assertEqualCopies(seq::take(n, seq::fromContainer(elements)));
        });

    newprop(
        "does not copy items",
        [] (std::vector<CopyGuard> elements) {
            std::size_t n = *newgen::inRange<std::size_t>(
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
    newprop(
        "drops all elements before the first element matching the predicate",
        [] (const std::vector<int> &elements, int limit) {
            const auto pred = [=](int x) { return x < limit; };
            const auto it = std::find_if(begin(elements), end(elements),
                                         [=](int x) { return !pred(x); });
            RC_ASSERT(seq::dropWhile(seq::fromContainer(elements), pred) ==
                      seq::fromIteratorRange(it, end(elements)));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &elements, int limit) {
            const auto pred = [=](int x) { return x < limit; };
            assertEqualCopies(seq::dropWhile(seq::fromContainer(elements), pred));
        });

    newprop(
        "does not copy items",
        [] (std::vector<CopyGuard> elements, int limit) {
            const auto pred = [=](const CopyGuard &x) { return x < limit; };
            auto seq = seq::dropWhile(seq::fromContainer(std::move(elements)), pred);
            while (seq.next());
        });

    SECTION("sanity check") {
        auto seq = seq::dropWhile(
            seq::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
            [](int x) { return x < 5; });
        auto expected = seq::just(5, 6, 7, 8, 9, 10);
        REQUIRE(seq == expected);
    }
}

TEST_CASE("seq::takeWhile") {
    newprop(
        "takes all elements before the first element matching the predicate",
        [] (const std::vector<int> &elements, int limit) {
            const auto pred = [=](int x) { return x < limit; };
            const auto it = std::find_if(begin(elements), end(elements),
                                         [=](int x) { return !pred(x); });
            RC_ASSERT(seq::takeWhile(seq::fromContainer(elements), pred) ==
                      seq::fromIteratorRange(begin(elements), it));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &elements, int limit) {
            const auto pred = [=](int x) { return x < limit; };
            assertEqualCopies(seq::takeWhile(seq::fromContainer(elements), pred));
        });

    newprop(
        "does not copy items",
        [] (std::vector<CopyGuard> elements, int limit) {
            const auto pred = [=](const CopyGuard &x) { return x < limit; };
            auto seq = seq::takeWhile(seq::fromContainer(std::move(elements)), pred);
            while (seq.next());
        });

    SECTION("sanity check") {
        auto seq = seq::takeWhile(
            seq::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
            [](int x) { return x < 5; });
        auto expected = seq::just(0, 1, 2, 3, 4);
        REQUIRE(seq == expected);
    }
}

TEST_CASE("seq::map") {
    newprop(
        "maps elements using mapping callable",
        [] (const std::vector<int> &elements, int x)
        {
            const auto mapper = [=](int a) { return a * x; };
            std::vector<int> expectedElements;
            for (const auto e : elements)
                expectedElements.push_back(mapper(e));
            auto mapSeq = seq::map(seq::fromContainer(elements), mapper);
            RC_ASSERT(mapSeq ==
                      seq::fromContainer(std::move(expectedElements)));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &elements, int x)
        {
            const auto mapper = [=](int a) { return a * x; };
            auto mapSeq = seq::map(seq::fromContainer(elements), mapper);
            assertEqualCopies(mapSeq);
        });

    newprop(
        "does not copy elements",
        [] (std::vector<CopyGuard> elements) {
            const auto mapper = [](CopyGuard &&x) { return std::move(x); };
            auto mapSeq = seq::map(
                seq::fromContainer(std::move(elements)), mapper);
            while (mapSeq.next());
        });
}

TEST_CASE("seq::zipWith") {
    newprop(
        "works with no sequences",
        [] (int x) {
            std::size_t n = *newgen::inRange<std::size_t>(0, 1000);
            const auto zipper = [=]{ return x; };
            auto zipSeq = seq::take(n, seq::zipWith(zipper));
            for (std::size_t i = 0; i < n; i++)
                RC_ASSERT(*zipSeq.next() == x);
            RC_ASSERT(!zipSeq.next());
        });

    newprop(
        "works with one sequence",
        [] (const std::vector<int> &elements, int x)
        {
            const auto zipper = [=](int a) { return a * x; };
            std::vector<int> expectedElements;
            for (const auto e : elements)
                expectedElements.push_back(zipper(e));
            auto zipSeq = seq::zipWith(zipper, seq::fromContainer(elements));
            RC_ASSERT(zipSeq ==
                      seq::fromContainer(std::move(expectedElements)));
        });

    newprop(
        "works with two sequences",
        [] (const std::vector<int> &e1,
            const std::vector<std::string> &e2)
        {
            const auto zipper = [](int a, std::string b) {
                return std::to_string(a) + b;
            };
            std::size_t size = std::min(e1.size(), e2.size());
            std::vector<std::string> expectedElements;
            for (std::size_t i = 0; i < size; i++)
                expectedElements.push_back(zipper(e1[i], e2[i]));
            auto zipSeq = seq::zipWith(zipper,
                                       seq::fromContainer(e1),
                                       seq::fromContainer(e2));
            RC_ASSERT(zipSeq ==
                      seq::fromContainer(std::move(expectedElements)));
        });

    newprop(
        "works with three sequences",
        [] (const std::vector<int> &e1,
            const std::vector<std::string> &e2,
            const std::vector<double> &e3)
        {
            const auto zipper = [](int a, std::string b, double c) {
                return std::to_string(a) + b + std::to_string(c);
            };
            std::size_t size = std::min({e1.size(), e2.size(), e3.size()});
            std::vector<std::string> expectedElements;
            for (std::size_t i = 0; i < size; i++)
                expectedElements.push_back(zipper(e1[i], e2[i], e3[i]));
            auto zipSeq = seq::zipWith(zipper,
                                       seq::fromContainer(e1),
                                       seq::fromContainer(e2),
                                       seq::fromContainer(e3));
            RC_ASSERT(zipSeq ==
                      seq::fromContainer(std::move(expectedElements)));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &e1,
            const std::vector<std::string> &e2,
            const std::vector<double> &e3)
        {
            const auto zipper = [](int a, std::string b, double c) {
                return std::to_string(a) + b + std::to_string(c);
            };
            auto zipSeq = seq::zipWith(zipper,
                                       seq::fromContainer(e1),
                                       seq::fromContainer(e2),
                                       seq::fromContainer(e3));
            assertEqualCopies(zipSeq);
        });

    newprop(
        "does not copy elements",
        [] (std::vector<CopyGuard> e1, std::vector<CopyGuard> e2) {
            const auto zipper = [](CopyGuard &&a, CopyGuard &&b) {
                return std::make_pair(std::move(a), std::move(b));
            };
            auto zipSeq = seq::zipWith(zipper,
                                       seq::fromContainer(std::move(e1)),
                                       seq::fromContainer(std::move(e2)));
            while (zipSeq.next());
        });
}

TEST_CASE("seq::filter") {
    newprop(
        "returns a seq with only the elements not matching the predicate"
        " removed",
        [] (const std::vector<int> &elements, int limit) {
            const auto pred = [=](int x) { return x < limit; };
            std::vector<int> expectedElements;
            std::copy_if(begin(elements), end(elements),
                         std::back_inserter(expectedElements),
                         pred);

            RC_ASSERT(seq::filter(seq::fromContainer(elements), pred) ==
                      seq::fromContainer(std::move(expectedElements)));
        });

    newprop(
        "copies are equal",
        [] (const std::vector<int> &elements, int limit) {
            const auto pred = [=](int x) { return x < limit; };
            std::vector<int> expectedElements;
            std::copy_if(begin(elements), end(elements),
                         std::back_inserter(expectedElements),
                         pred);
            assertEqualCopies(seq::filter(seq::fromContainer(elements), pred));
        });

    newprop(
        "does not copy elements",
        [] (std::vector<CopyGuard> elements, int limit) {
            const auto pred = [=](const CopyGuard &x) { return x.value < limit; };
            auto seq = seq::filter(seq::fromContainer(std::move(elements)), pred);
            while (seq.next());
        });
}

TEST_CASE("seq::join") {
    static const auto subgen = newgen::scale(
        0.25, newgen::arbitrary<std::vector<int>>());
    static const auto gen = newgen::container<std::vector<std::vector<int>>>(subgen);

    newprop(
        "returns a seq with the elements of all seqs joined together",
        []{
            auto vectors = *gen;
            auto seqs = seq::map(
                seq::fromContainer(vectors),
                [](std::vector<int> &&vec) {
                    return seq::fromContainer(std::move(vec));
                });

            std::vector<int> expectedElements;
            for (const auto &vec : vectors) {
                expectedElements.insert(expectedElements.end(),
                                        begin(vec), end(vec));
            }

            RC_ASSERT(seq::join(seqs) == seq::fromContainer(expectedElements));
        });

    newprop(
        "copies are equal",
        []{
            auto vectors = *gen;
            auto seqs = seq::map(
                seq::fromContainer(vectors),
                [](std::vector<int> &&vec) {
                    return seq::fromContainer(std::move(vec));
                });
            assertEqualCopies(seq::join(seqs));
        });

    newprop(
        "does not copy elements",
        []{
            static const auto subguardgen =
                newgen::scale(0.25, newgen::arbitrary<std::vector<CopyGuard>>());
            static const auto guardgen =
                newgen::container<std::vector<std::vector<CopyGuard>>>(
                    subguardgen);
            auto vectors = *guardgen;
            auto seqs = seq::map(
                seq::fromContainer(std::move(vectors)),
                [](std::vector<CopyGuard> &&vec) {
                    return seq::fromContainer(std::move(vec));
                });
            auto seq = seq::join(std::move(seqs));
            while (seq.next());
        });
}

TEST_CASE("seq::concat") {
    newprop(
        "joins the given sequences together",
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

    newprop(
        "copies are equal",
        [] (const std::vector<int> &a,
            const std::vector<int> &b,
            const std::vector<int> &c)
        {
            assertEqualCopies(seq::concat(seq::fromContainer(a),
                                          seq::fromContainer(b),
                                          seq::fromContainer(c)));
        });

    newprop(
        "does not copy elements",
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
    newprop(
        "equivalent to seq::join(seq::map(...))",
        [](std::vector<int> elements) {
            const auto seq = seq::fromContainer(std::move(elements));
            const auto mapper = [](int a) {
                return seq::just(a, a + 1, a + 2);
            };

            RC_ASSERT(seq::mapcat(seq, mapper) ==
                      seq::join(seq::map(seq, mapper)));
        });

    newprop(
        "copies are equal",
        [](std::vector<int> elements) {
            const auto seq = seq::fromContainer(std::move(elements));
            const auto mapper = [](int a) {
                return seq::just(a, a + 1, a + 2);
            };

            assertEqualCopies(seq::mapcat(seq, mapper));
        });

    newprop(
        "does not copy elements",
        [](std::vector<CopyGuard> elements) {
            auto seq = seq::fromContainer(std::move(elements));
            const auto mapper = [](CopyGuard &&a) {
                return seq::just(std::move(a), CopyGuard(1337));
            };
            auto mapSeq = seq::mapcat(std::move(seq), mapper);
            while (mapSeq.next());
        });
}

TEST_CASE("seq::mapMaybe") {
    newprop(
        "removes elements for which mapper evaluates to Nothing",
        [](const std::vector<int> &elements) {
            const auto seq = seq::fromContainer(elements);
            const auto pred = [](int x) { return (x % 2) == 0; };
            const auto maybeSeq = seq::mapMaybe(seq, [=](int x) -> Maybe<int> {
                return pred(x) ? Maybe<int>(x) : Nothing;
            });
            RC_ASSERT(maybeSeq == seq::filter(seq, pred));
        });

    newprop(
        "for elements that are not Nothing, unwraps the contents",
        [](const std::vector<int> &elements) {
            const auto seq = seq::fromContainer(elements);
            const auto mapper = [](int x) { return x * x; };
            const auto maybeSeq = seq::mapMaybe(seq, [=](int x) -> Maybe<int> {
                return mapper(x);
            });
            RC_ASSERT(maybeSeq == seq::map(seq, mapper));
        });

    newprop(
        "copies are equal",
        [](const std::vector<int> &elements) {
            const auto seq = seq::fromContainer(elements);
            assertEqualCopies(
                seq::mapMaybe(seq, [=](int x) -> Maybe<int> {
                    return x * x;
                }));
        });

    newprop(
        "does not copy elements",
        [](std::vector<CopyGuard> elements) {
            auto seq = seq::fromContainer(std::move(elements));
            auto maybeSeq = seq::mapMaybe(
                std::move(seq),
                [=](CopyGuard x) -> Maybe<CopyGuard> {
                    return std::move(x);
                });
            while (maybeSeq.next());
        });
}

TEST_CASE("seq::cycle") {
    newprop(
        "returns an infinite cycle of the given Seq",
        []{
            auto elements = *newgen::suchThat<std::vector<int>>(
                [](const std::vector<int> &x) { return !x.empty(); });
            auto seq = seq::cycle(seq::fromContainer(elements));
            auto it = begin(elements);
            for (int i = 0; i < 2000; i++) {
                RC_ASSERT(*seq.next() == *it++);
                if (it == end(elements))
                    it = begin(elements);
            }
        });

    newprop(
        "does not copy Seq on construction",
        []{
            auto elements = *newgen::suchThat<std::vector<CopyGuard>>(
                [](const std::vector<CopyGuard> &x) { return !x.empty(); });
            auto seq = seq::cycle(seq::fromContainer(std::move(elements)));
        });

    newprop(
        "copies are equal",
        []{
            auto elements = *newgen::suchThat<std::vector<int>>(
                [](const std::vector<int> &x) { return !x.empty(); });
            auto seq = seq::cycle(seq::fromContainer(elements));
            assertEqualCopies(seq::take(1000, std::move(seq)));
        });
}

TEST_CASE("seq::cast") {
    newprop(
        "casting to a larger type and then back yields Seq equal to original",
        [](const std::vector<uint8_t> &elements) {
            const auto seq = seq::fromContainer(elements);
            RC_ASSERT(seq::cast<uint8_t>(seq::cast<int>(seq)) == seq);
        });

    newprop(
        "copies are equal",
        [](const std::vector<uint8_t> &elements) {
            assertEqualCopies(
                seq::cast<int>(seq::fromContainer(elements)));
        });

    newprop(
        "does not copy elements",
        [](std::vector<CopyGuard> elements) {
            auto seq = seq::cast<CopyGuard>(
                seq::fromContainer(std::move(elements)));
            while (seq.next());
        });
}
