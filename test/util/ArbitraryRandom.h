#pragma once

// TODO clean up, formalize

namespace rc {
namespace test {

struct RandomSequence
{
    std::vector<bool> splits;
    uint64_t nexts;
};

inline void showValue(const RandomSequence &seq, std::ostream &os)
{
    os << "splits(" << seq.splits.size() << ")=";
    show(seq.splits, os);
    os << ", nexts=" << seq.nexts;
}

inline void runRandomSequence(const RandomSequence &seq, Random &random)
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

} // namespace test

template<>
class Arbitrary<test::RandomSequence>
    : public gen::Generator<test::RandomSequence>
{
public:
    test::RandomSequence generate() const override
    {
        auto values = *gen::pairOf(
            gen::arbitrary<std::vector<bool>>(),
            gen::ranged<uint64_t>(0, 10000));
        test::RandomSequence seq;
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
        runRandomSequence(*gen::arbitrary<test::RandomSequence>(), random);
        return random;
    }
};

} // namespace rc
