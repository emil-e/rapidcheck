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


// This will only generate Randoms with no next() called.
template<>
struct NewArbitrary<Random>
{
    static Gen<Random> arbitrary()
    {
        return newgen::map(
            newgen::pair(newgen::arbitrary<Random::Key>(),
                         newgen::arbitrary<std::vector<bool>>()),
            [](const std::pair<Random::Key, std::vector<bool>> &p) {
                Random random(p.first);
                for (bool x : p.second) {
                    if (!x)
                        random.split();
                    else
                        random = random.split();
                }
                return random;
            });
    }
};


namespace test {

// This will also generate Randoms where next() has been called
inline Gen<Random> trulyArbitraryRandom()
{
    return newgen::map(
        newgen::pair(newgen::arbitrary<Random>(),
                     newgen::inRange<int>(0, 10000)),
        [](std::pair<Random, int> &&p){
            auto nexts = p.second;
            while (nexts-- > 0)
                p.first.next();
            return p.first;
        });
}

} // namespace test
} // namespace rc
