#pragma once

// TODO clean up, formalize

namespace rc {

// This will only generate Randoms with no next() called.
template<>
class Arbitrary<Random> : public gen::Generator<Random>
{
public:
    Random generate() const override
    {
        Random random(*gen::arbitrary<Random::Key>());
        auto splits = *gen::arbitrary<std::vector<bool>>();
        for (bool x : splits) {
            if (!x)
                random.split();
            else
                random = random.split();
        }
        return random;
    }
};

namespace test {

// This will also generate Randoms where next() has been called
class TrulyArbitraryRandom : public gen::Generator<Random>
{
public:
    Random generate() const override
    {
        auto random = *gen::arbitrary<Random>();
        auto nexts = *gen::ranged<int>(0, 10000);
        while (nexts-- > 0)
            random.next();
        return random;
    }
};

} // namespace test
} // namespace rc
