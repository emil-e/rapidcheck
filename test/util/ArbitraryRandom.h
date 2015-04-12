#pragma once

// TODO clean up, formalize

namespace rc {

// This will only generate Randoms with no next() called.
template<>
struct Arbitrary<Random>
{
    static Gen<Random> arbitrary()
    {
        return gen::map(
            gen::pair(gen::arbitrary<Random::Key>(),
                         gen::arbitrary<std::vector<bool>>()),
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
    return gen::map(
        gen::pair(gen::arbitrary<Random>(),
                     gen::inRange<int>(0, 10000)),
        [](std::pair<Random, int> &&p){
            auto nexts = p.second;
            while (nexts-- > 0)
                p.first.next();
            return p.first;
        });
}

} // namespace test
} // namespace rc
