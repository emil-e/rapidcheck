#include <catch.hpp>
#include <rapidcheck/catch.h>

#include <numeric>

#include "rapidcheck/detail/BitStream.h"
#include "rapidcheck/detail/Utility.h"

using namespace rc;
using namespace rc::detail;

template <typename T>
class SeqSource {
public:
  template <typename Arg,
            typename = typename std::enable_if<
                !std::is_same<Decay<Arg>, SeqSource>::value>::type>
  SeqSource(Arg &&arg)
      : m_seq(std::forward<Arg>(arg))
      , m_requested(0) {}

  T next() {
    auto value = m_seq.next();
    RC_ASSERT(value);
    m_requested++;
    return *value;
  }

  int requested() const { return m_requested; }

private:
  Seq<T> m_seq;
  int m_requested;
};

template <typename T>
SeqSource<T> makeSource(Seq<T> seq) {
  return SeqSource<T>(std::move(seq));
}

TEST_CASE("BitStream") {
  SECTION("next") {
    const auto bitSizes =
        gen::container<std::vector<int>>(gen::inRange(0, 100));

    prop("requests the correct number of bits",
         [=] {
           auto source = makeSource(seq::repeat('\xAB'));
           auto stream = bitStreamOf(source);

           const auto sizes = *bitSizes;
           int totalSize = 0;
           for (int size : sizes) {
             totalSize += size;
             stream.next<uint64_t>(size);
           }

           int expected = totalSize / 8;
           if ((totalSize % 8) != 0) {
             expected++;
           }
           RC_ASSERT(source.requested() == expected);
         });

    prop("spills no bits",
         [=](uint64_t x) {
           auto source = makeSource(seq::repeat(x));
           auto stream = bitStreamOf(source);

           const auto sizes = *gen::suchThat(bitSizes,
                                       [](const std::vector<int> &x) {
                                         return std::accumulate(
                                                    begin(x), end(x), 0) >= 64;
                                       });

           uint64_t value = 0;
           int n = 64;
           for (int size : sizes) {
             if (n == 0) {
               break;
             }
             const auto r = std::min(n, size);
             const auto bits = stream.next<uint64_t>(r);
             value |= (bits << (64ULL - n));
             n -= r;
           }

           RC_ASSERT(value == x);
         });

    prop("takes multiple values per request if required to fill result",
         [](uint8_t byte) {
           auto source = makeSource(seq::take(8, seq::repeat(byte)));
           auto stream = bitStreamOf(source);
           uint64_t value = stream.next<uint64_t>(64);
           uint64_t expected = 0;
           for (int i = 0; i < 8; i++) {
             expected <<= 8ULL;
             expected |= byte;
           }

           RC_ASSERT(value == expected);
         });

    prop("does not return more bits than requested (unsigned)",
         [=](uint64_t x) {
           auto source = makeSource(seq::repeat(x));
           auto stream = bitStreamOf(source);
           int n = *gen::inRange(0, 64);
           RC_ASSERT((stream.next<uint64_t>(n) & ~bitMask<uint64_t>(n)) == 0);
         });

    prop("does not return more bits than requested (signed)",
         [=](uint64_t x) {
           auto source = makeSource(seq::repeat(x));
           auto stream = bitStreamOf(source);
           const auto n = *gen::inRange(0, 64);
           const bool sign = (x & (1LL << (n - 1LL))) != 0;
           const auto mask = ~bitMask<int64_t>(n);
           if (sign) {
             RC_ASSERT((stream.next<int64_t>(n) & mask) == mask);
           } else {
             RC_ASSERT((stream.next<int64_t>(n) & mask) == 0);
           }
         });

    prop("works with booleans",
         [](uint64_t x) {
           auto source = makeSource(seq::just(x));
           auto stream = bitStreamOf(source);
           uint64_t value = 0;
           for (uint64_t i = 0; i < 64ULL; i++) {
             if (stream.next<bool>(1)) {
               value |= 1ULL << i;
             }
           }

           RC_ASSERT(value == x);
         });
  }

  SECTION("nextWithSize") {
    prop("requests full number of bits for kNominalSize",
         [=] {
           auto source = makeSource(seq::repeat('\xAB'));
           auto stream = bitStreamOf(source);

           int n = *gen::inRange(0, 100);
           for (int i = 0; i < n; i++) {
             stream.nextWithSize<char>(kNominalSize);
           }

           RC_ASSERT(source.requested() == n);
         });

    prop("requests half number of bits for kNominalSize / 2",
         [=] {
           auto source = makeSource(seq::repeat('\xAB'));
           auto stream = bitStreamOf(source);

           int n = *gen::suchThat(gen::inRange(0, 100),
                                  [](int x) { return (x % 2) == 0; });

           for (int i = 0; i < n; i++) {
             stream.nextWithSize<char>(kNominalSize / 2);
           }

           RC_ASSERT(source.requested() == (n / 2));
         });

    prop("requests no bits for size 0",
         [=] {
           auto source = makeSource(seq::repeat('\xAB'));
           auto stream = bitStreamOf(source);

           int n = *gen::inRange(0, 100);
           for (int i = 0; i < n; i++) {
             stream.nextWithSize<char>(0);
           }

           RC_ASSERT(source.requested() == 0);
         });
  }

  SECTION("bitStreamOf") {
    SECTION("copies source if const") {
      auto source = makeSource(seq::just(0, 1));
      const auto &constSource = source;
      auto stream = bitStreamOf(constSource);
      stream.next<int>();
      REQUIRE(source.next() == 0);
    }

    SECTION("references source if non-const") {
      auto source = makeSource(seq::just(0, 1));
      auto &nonConstSource = source;
      auto stream = bitStreamOf(nonConstSource);
      stream.next<int>();
      REQUIRE(source.next() == 1);
    }
  }
}
