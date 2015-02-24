#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/Random.h"

using namespace rc;

struct Sequence
{
    std::vector<bool> splits;
    uint64_t nexts;
};

void showValue(const Sequence &seq, std::ostream &os)
{
    os << "splits(" << seq.splits.size() << ")=";
    show(seq.splits, os);
    os << ", nexts=" << seq.nexts;
}

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

template<>
class Arbitrary<Random> : public gen::Generator<Random>
{
public:
    Random generate() const override
    {
        Random random(*gen::arbitrary<Random::Key>());
        runSequence(*gen::arbitrary<Sequence>(), random);
        return random;
    }
};

} // namespace rc

TEST_CASE("Random") {
    prop("different keys yield inequal generators",
         [] {
             auto key1 = *gen::arbitrary<Random::Key>();
             auto key2 = *gen::distinctFrom(key1);
             RC_ASSERT(Random(key1) != Random(key2));
         });

    prop("different splits are inequal",
         [](Random r1) {
             Random r2(r1.split());
             RC_ASSERT(r1 != r2);
         });

    prop("next modifies state",
         [](Random r1) {
             Random r2(r1);
             r2.next();
             RC_ASSERT(r1 != r2);
         });

    prop("different keys yield different sequences",
         [] {
             auto key1 = *gen::arbitrary<Random::Key>();
             auto key2 = *gen::distinctFrom(key1);
             Random r1(key1);
             Random r2(key2);
             for (std::size_t i = 0; i < 4; i++)
                 RC_SUCCEED_IF(r1.next() != r2.next());
             RC_FAIL("Equal random numbers");
         });

    prop("different splits yield different sequences",
         [](Random r1) {
             Random r2(r1.split());

             // Random numbers are generated in 256-bit blocks so if the first
             // four are equal, it's likely the same block and something is
             // wrong even if subsequent blocks are not equal
             for (std::size_t i = 0; i < 4; i++)
                 RC_SUCCEED_IF(r1.next() != r2.next());
             RC_FAIL("Equal random numbers");
         });

    prop("different number of nexts yield different sequences",
         [](Random r1) {
             Random r2(r1);
             auto gen = gen::ranged<int>(0, 2000);
             int n1 = *gen;
             int n2 = *gen::distinctFrom(gen, n1);

             for (std::size_t i = 0; i < n1; i++)
                 r1.next();
             for (std::size_t i = 0; i < n2; i++)
                 r2.next();

             for (std::size_t i = 0; i < 4; i++)
                 RC_SUCCEED_IF(r1.next() != r2.next());
             RC_FAIL("Equal random numbers");
         });

    prop("copies are equal",
         [](Random r1) {
             Random r2(r1);
             RC_ASSERT(r1 == r2);
         });

    prop("copies yield equal random numbers",
         [](Random r1) {
             Random r2(r1);

             // The first N numbers could theoretically be equal even if
             // subsequent numbers are not but the likelihood of different
             // states generating identical 64 * N bits is very low.
             for (std::size_t i = 0; i < 1000; i++) {
                 auto x1 = r1.next();
                 RC_ASSERT(x1 == r2.next());
             }
         });

    // Somewhat dubious test, I know. Hopefully, if something totally breaks,
    // it'll let us know at least.
    prop("has uniform distribution",
         [](Random random) {
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
