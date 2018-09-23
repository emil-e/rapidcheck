#include <catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/seq/Create.h"
#include "rapidcheck/seq/Operations.h"

#include "util/SeqUtils.h"
#include "util/Logger.h"

using namespace rc;
using namespace rc::test;

TEST_CASE("seq::drop") {
  prop("drops the first N elements from the given seq",
       [](const std::vector<int> &elements) {
         std::size_t n =
             *gen::inRange<std::size_t>(0, (elements.size() + 1) * 2);
         std::size_t start = std::min(elements.size(), n);
         std::vector<int> rest(elements.begin() + start, elements.end());
         RC_ASSERT(seq::drop(n, seq::fromContainer(elements)) ==
                   seq::fromContainer(rest));
       });

  prop("copies are equal",
       [](const std::vector<int> &elements) {
         std::size_t n =
             *gen::inRange<std::size_t>(0, (elements.size() + 1) * 2);
         assertEqualCopies(seq::drop(n, seq::fromContainer(elements)));
       });

  SECTION("does not copy items") {
    auto seq = seq::drop(2, seq::just(Logger(), Logger(), Logger(), Logger()));
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }

  SECTION("sanity check") {
    REQUIRE(seq::drop(2, seq::just(1, 2, 3)) == seq::just(3));
  }
}

TEST_CASE("seq::take") {
  prop("takes the first N elements from the given seq",
       [](const std::vector<int> &elements) {
         std::size_t n =
             *gen::inRange<std::size_t>(0, (elements.size() + 1) * 2);
         std::size_t start = std::min(elements.size(), n);
         std::vector<int> head(elements.begin(), elements.begin() + start);
         RC_ASSERT(seq::take(n, seq::fromContainer(elements)) ==
                   seq::fromContainer(head));
       });

  prop("copies are equal",
       [](const std::vector<int> &elements) {
         std::size_t n =
             *gen::inRange<std::size_t>(0, (elements.size() + 1) * 2);
         assertEqualCopies(seq::take(n, seq::fromContainer(elements)));
       });

  SECTION("does not copy items") {
    auto seq = seq::take(2, seq::just(Logger(), Logger(), Logger(), Logger()));
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }

  SECTION("sanity check") {
    REQUIRE(seq::take(2, seq::just(1, 2, 3)) == seq::just(1, 2));
  }
}

TEST_CASE("seq::dropWhile") {
  prop("drops all elements before the first element matching the predicate",
       [](const std::vector<int> &elements, int limit) {
         const auto pred = [=](int x) { return x < limit; };
         const auto it = std::find_if(
             begin(elements), end(elements), [=](int x) { return !pred(x); });
         RC_ASSERT(seq::dropWhile(seq::fromContainer(elements), pred) ==
                   seq::fromIteratorRange(it, end(elements)));
       });

  prop("copies are equal",
       [](const std::vector<int> &elements, int limit) {
         const auto pred = [=](int x) { return x < limit; };
         assertEqualCopies(seq::dropWhile(seq::fromContainer(elements), pred));
       });

  SECTION("does not copy items") {
    const auto pred = [=](const Logger &x) { return x.id == "drop"; };
    auto seq = seq::dropWhile(
        seq::just(Logger("drop"), Logger("drop"), Logger(), Logger()), pred);
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }

  SECTION("sanity check") {
    auto seq = seq::dropWhile(seq::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
                              [](int x) { return x < 5; });
    auto expected = seq::just(5, 6, 7, 8, 9, 10);
    REQUIRE(seq == expected);
  }
}

TEST_CASE("seq::takeWhile") {
  prop("takes all elements before the first element matching the predicate",
       [](const std::vector<int> &elements, int limit) {
         const auto pred = [=](int x) { return x < limit; };
         const auto it = std::find_if(
             begin(elements), end(elements), [=](int x) { return !pred(x); });
         RC_ASSERT(seq::takeWhile(seq::fromContainer(elements), pred) ==
                   seq::fromIteratorRange(begin(elements), it));
       });

  prop("copies are equal",
       [](const std::vector<int> &elements, int limit) {
         const auto pred = [=](int x) { return x < limit; };
         assertEqualCopies(seq::takeWhile(seq::fromContainer(elements), pred));
       });

  SECTION("does not copy items") {
    const auto pred = [=](const Logger &x) { return x.id == "take"; };
    auto seq = seq::takeWhile(
        seq::just(Logger("take"), Logger("take"), Logger(), Logger()), pred);
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }

  SECTION("sanity check") {
    auto seq = seq::takeWhile(seq::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
                              [](int x) { return x < 5; });
    auto expected = seq::just(0, 1, 2, 3, 4);
    REQUIRE(seq == expected);
  }
}

TEST_CASE("seq::map") {
  prop("maps elements using mapping callable",
       [](const std::vector<int> &elements, int x) {
         const auto mapper = [=](int a) { return a * x; };
         std::vector<int> expectedElements;
         for (const auto e : elements) {
           expectedElements.push_back(mapper(e));
         }
         auto mapSeq = seq::map(seq::fromContainer(elements), mapper);
         RC_ASSERT(mapSeq == seq::fromContainer(std::move(expectedElements)));
       });

  prop("copies are equal",
       [](const std::vector<int> &elements, int x) {
         const auto mapper = [=](int a) { return a * x; };
         auto mapSeq = seq::map(seq::fromContainer(elements), mapper);
         assertEqualCopies(mapSeq);
       });

  SECTION("does not copy elements") {
    const auto mapper = [](Logger &&x) { return std::move(x); };
    auto seq = seq::map(seq::just(Logger(), Logger()), mapper);
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }
}

TEST_CASE("seq::zipWith") {
  prop("works with no sequences",
       [](int x) {
         std::size_t n = *gen::inRange<std::size_t>(0, 1000);
         const auto zipper = [=] { return x; };
         auto zipSeq = seq::take(n, seq::zipWith(zipper));
         for (std::size_t i = 0; i < n; i++) {
           RC_ASSERT(*zipSeq.next() == x);
         }
         RC_ASSERT(!zipSeq.next());
       });

  prop("works with one sequence",
       [](const std::vector<int> &elements, int x) {
         const auto zipper = [=](int a) { return a * x; };
         std::vector<int> expectedElements;
         for (const auto e : elements) {
           expectedElements.push_back(zipper(e));
         }
         auto zipSeq = seq::zipWith(zipper, seq::fromContainer(elements));
         RC_ASSERT(zipSeq == seq::fromContainer(std::move(expectedElements)));
       });

  prop("works with two sequences",
       [](const std::vector<int> &e1, const std::vector<std::string> &e2) {
         const auto zipper =
             [](int a, std::string b) { return std::to_string(a) + b; };
         std::size_t size = std::min(e1.size(), e2.size());
         std::vector<std::string> expectedElements;
         for (std::size_t i = 0; i < size; i++) {
           expectedElements.push_back(zipper(e1[i], e2[i]));
         }
         auto zipSeq = seq::zipWith(
             zipper, seq::fromContainer(e1), seq::fromContainer(e2));
         RC_ASSERT(zipSeq == seq::fromContainer(std::move(expectedElements)));
       });

  prop("works with three sequences",
       [](const std::vector<int> &e1,
          const std::vector<std::string> &e2,
          const std::vector<double> &e3) {
         const auto zipper = [](int a, std::string b, double c) {
           return std::to_string(a) + b + std::to_string(c);
         };
         std::size_t size = std::min({e1.size(), e2.size(), e3.size()});
         std::vector<std::string> expectedElements;
         for (std::size_t i = 0; i < size; i++) {
           expectedElements.push_back(zipper(e1[i], e2[i], e3[i]));
         }
         auto zipSeq = seq::zipWith(zipper,
                                    seq::fromContainer(e1),
                                    seq::fromContainer(e2),
                                    seq::fromContainer(e3));
         RC_ASSERT(zipSeq == seq::fromContainer(std::move(expectedElements)));
       });

  prop("copies are equal",
       [](const std::vector<int> &e1,
          const std::vector<std::string> &e2,
          const std::vector<double> &e3) {
         const auto zipper = [](int a, std::string b, double c) {
           return std::to_string(a) + b + std::to_string(c);
         };
         auto zipSeq = seq::zipWith(zipper,
                                    seq::fromContainer(e1),
                                    seq::fromContainer(e2),
                                    seq::fromContainer(e3));
         assertEqualCopies(zipSeq);
       });

  SECTION("does not copy elements") {
    const auto zipper = [](Logger &&a, Logger &&b) {
      return std::make_pair(std::move(a), std::move(b));
    };
    auto seq = seq::zipWith(zipper,
                            seq::just(Logger(), Logger(), Logger()),
                            seq::just(Logger(), Logger(), Logger()));
    while (const auto p = seq.next()) {
      REQUIRE(p->first.numberOf("copy") == 0);
      REQUIRE(p->second.numberOf("copy") == 0);
    }
  }
}

TEST_CASE("seq::filter") {
  prop(
      "returns a seq with only the elements not matching the predicate"
      " removed",
      [](const std::vector<int> &elements, int limit) {
        const auto pred = [=](int x) { return x < limit; };
        std::vector<int> expectedElements;
        std::copy_if(begin(elements),
                     end(elements),
                     std::back_inserter(expectedElements),
                     pred);

        RC_ASSERT(seq::filter(seq::fromContainer(elements), pred) ==
                  seq::fromContainer(std::move(expectedElements)));
      });

  prop("copies are equal",
       [](const std::vector<int> &elements, int limit) {
         const auto pred = [=](int x) { return x < limit; };
         std::vector<int> expectedElements;
         std::copy_if(begin(elements),
                      end(elements),
                      std::back_inserter(expectedElements),
                      pred);
         assertEqualCopies(seq::filter(seq::fromContainer(elements), pred));
       });

  SECTION("does not copy elements") {
    const auto pred = [=](const Logger &x) { return x.id != "*"; };
    auto seq = seq::filter(
        seq::just(Logger(), Logger("*"), Logger(), Logger("*")), pred);
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }
}

TEST_CASE("seq::join") {
  static const auto subgen =
      gen::scale(0.25, gen::arbitrary<std::vector<int>>());
  static const auto gen = gen::container<std::vector<std::vector<int>>>(subgen);

  prop("returns a seq with the elements of all seqs joined together",
       [] {
         auto vectors = *gen;
         auto seqs = seq::map(seq::fromContainer(vectors),
                              [](std::vector<int> &&vec) {
                                return seq::fromContainer(std::move(vec));
                              });

         std::vector<int> expectedElements;
         for (const auto &vec : vectors) {
           expectedElements.insert(
               expectedElements.end(), begin(vec), end(vec));
         }

         RC_ASSERT(seq::join(seqs) == seq::fromContainer(expectedElements));
       });

  prop("copies are equal",
       [] {
         auto vectors = *gen;
         auto seqs = seq::map(seq::fromContainer(vectors),
                              [](std::vector<int> &&vec) {
                                return seq::fromContainer(std::move(vec));
                              });
         assertEqualCopies(seq::join(seqs));
       });

  SECTION("does not copy elements") {
    auto seq = seq::join(seq::just(seq::just(Logger(), Logger()),
                                   seq::just(Logger(), Logger())));
    while (const auto value = seq.next()) {
      REQUIRE(value->numberOf("copy") == 0);
    }
  }
}

TEST_CASE("seq::concat") {
  prop("joins the given sequences together",
       [](const std::vector<int> &a,
          const std::vector<int> &b,
          const std::vector<int> &c) {
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
       [](const std::vector<int> &a,
          const std::vector<int> &b,
          const std::vector<int> &c) {
         assertEqualCopies(seq::concat(seq::fromContainer(a),
                                       seq::fromContainer(b),
                                       seq::fromContainer(c)));
       });

  SECTION("does not copy elements") {
    auto seq = seq::concat(seq::just(Logger(), Logger()),
                           seq::just(Logger(), Logger()),
                           seq::just(Logger(), Logger()));
    while (const auto value = seq.next()) {
      REQUIRE(value->numberOf("copy") == 0);
    }
  }
}

TEST_CASE("seq::mapcat") {
  prop("equivalent to seq::join(seq::map(...))",
       [](std::vector<int> elements) {
         const auto seq = seq::fromContainer(std::move(elements));
         const auto mapper = [](int a) { return seq::just(a, a + 1, a + 2); };

         RC_ASSERT(seq::mapcat(seq, mapper) ==
                   seq::join(seq::map(seq, mapper)));
       });

  prop("copies are equal",
       [](std::vector<int> elements) {
         const auto seq = seq::fromContainer(std::move(elements));
         const auto mapper = [](int a) { return seq::just(a, a + 1, a + 2); };

         assertEqualCopies(seq::mapcat(seq, mapper));
       });

  SECTION("does not copy elements") {
    auto seq = seq::just(Logger(), Logger());
    const auto mapper =
        [](Logger &&a) { return seq::just(std::move(a), Logger("extra")); };
    auto mapSeq = seq::mapcat(std::move(seq), mapper);
    while (const auto value = mapSeq.next()) {
      REQUIRE(value->numberOf("copy") == 0);
    }
  }
}

TEST_CASE("seq::mapMaybe") {
  prop("removes elements for which mapper evaluates to Nothing",
       [](const std::vector<int> &elements) {
         const auto seq = seq::fromContainer(elements);
         const auto pred = [](int x) { return (x % 2) == 0; };
         const auto maybeSeq =
             seq::mapMaybe(seq,
                           [=](int x) -> Maybe<int> {
                             return pred(x) ? Maybe<int>(x) : Nothing;
                           });
         RC_ASSERT(maybeSeq == seq::filter(seq, pred));
       });

  prop("for elements that are not Nothing, unwraps the contents",
       [](const std::vector<int> &elements) {
         const auto seq = seq::fromContainer(elements);
         const auto mapper = [](int x) { return x * x; };
         const auto maybeSeq =
             seq::mapMaybe(seq, [=](int x) -> Maybe<int> { return mapper(x); });
         RC_ASSERT(maybeSeq == seq::map(seq, mapper));
       });

  prop("copies are equal",
       [](const std::vector<int> &elements) {
         const auto seq = seq::fromContainer(elements);
         assertEqualCopies(
             seq::mapMaybe(seq, [=](int x) -> Maybe<int> { return x * x; }));
       });

  SECTION("does not copy elements") {
    auto seq = seq::just(Logger(), Logger(), Logger());
    auto maybeSeq =
        seq::mapMaybe(std::move(seq),
                      [=](Logger x) -> Maybe<Logger> { return std::move(x); });
    while (const auto value = maybeSeq.next()) {
      REQUIRE(value->numberOf("copy") == 0);
    }
  }
}

TEST_CASE("seq::cycle") {
  prop("returns an infinite cycle of the given Seq",
       [] {
         auto elements = *gen::nonEmpty<std::vector<int>>();
         auto seq = seq::cycle(seq::fromContainer(elements));
         auto it = begin(elements);
         for (int i = 0; i < 2000; i++) {
           RC_ASSERT(*seq.next() == *it++);
           if (it == end(elements)) {
             it = begin(elements);
           }
         }
       });

  SECTION("does not copy Seq on construction") {
    auto seq = seq::cycle(seq::just(Logger(), Logger()));
    REQUIRE(seq.next()->numberOf("copy") <= 1);
    REQUIRE(seq.next()->numberOf("copy") <= 1);
  }

  prop("copies are equal",
       [] {
         auto elements = *gen::nonEmpty<std::vector<int>>();
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
         assertEqualCopies(seq::cast<int>(seq::fromContainer(elements)));
       });

  SECTION("does not copy elements") {
    auto seq = seq::cast<Logger>(seq::just(Logger(), Logger()));
    REQUIRE(seq.next()->numberOf("copy") == 0);
    REQUIRE(seq.next()->numberOf("copy") == 0);
  }
}
