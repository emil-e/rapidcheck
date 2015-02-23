#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/Random.h"

#define ENOUGH 1000

using namespace rc;

struct Sequence
{
    std::vector<bool> splits;
    uint64_t nexts;
};

void runSequence(const Sequence &seq, Random &random)
{
    for (bool x : seq.splits) {
        if (!x)
            random.split();
        else
            random = random.split();
    }

    for (uint64_t i = 0; i < seq.nexts; i++)
        random.next();
}

namespace rc {

template<>
class Arbitrary<Sequence> : public gen::Generator<Sequence>
{
public:
    Sequence generate() const override
    {
        auto values = *gen::pairOf(
            gen::arbitrary<std::vector<bool>>(),
            gen::ranged<uint64_t>(0, 10000));
        Sequence seq;
        seq.splits = values.first;
        seq.nexts = values.second;
        return seq;
    }
};

} // namespace rc

TEST_CASE("Random") {
    prop("different splits yield different sequences",
         [] {
             auto seqs = *gen::suchThat(
                 gen::arbitrary<std::pair<Sequence, Sequence>>(),
                 [&](const std::pair<Sequence, Sequence> &s) {
                     // Must be prefix-free
                     std::size_t size = std::min(s.first.splits.size(),
                                                 s.second.splits.size());
                     return !std::equal(begin(s.first.splits),
                                        begin(s.first.splits) + size,
                                        begin(s.second.splits));
                 });

             Random r1(*gen::arbitrary<Random::Key>());
             Random r2(r1);
             runSequence(seqs.first, r1);
             runSequence(seqs.second, r2);

             for (int i = 0; i < ENOUGH; i++)
                 RC_SUCCEED_IF(r1.next() != r2.next());
             RC_FAIL("Equal random numbers");
         });

    prop("copies yield equal random numbers",
         [](const Sequence &seq) {
             Random random(*gen::arbitrary<Random::Key>());
             runSequence(seq, random);

             Random r1(random);
             Random r2(random);

             for (int i = 0; i < ENOUGH; i++) {
                 auto x1 = r1.next();
                 RC_ASSERT(x1 == r2.next());
             }
         });

    // Somewhat dubious test, I know. Hopefully, if something totally breaks,
    // it'll let us know at least.
    prop("has uniform distribution",
         [](const Sequence &seq) {
             Random random(*gen::arbitrary<Random::Key>());
             runSequence(seq, random);

             std::array<uint64_t, 16> bins;
             bins.fill(0);
             const std::size_t nSamples = 200000;
             for (std::size_t i = 0; i < nSamples; i++)
                 bins[random.next() % bins.size()]++;

             double ideal = nSamples / static_cast<double>(bins.size());
             double error = 0.0;
             for (auto x : bins) {
                 double diff = 1.0 - (x / ideal);
                 error += (diff * diff);
             }

             RC_ASSERT(error < 0.01);
         });
}
