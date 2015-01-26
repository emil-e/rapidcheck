#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/RandomEngine.h"

using namespace rc;
using namespace rc::detail;

#define ENOUGH 10000

TEST_CASE("RandomEngine") {
    SECTION("nextAtom") {
        prop("same seeds yields same sequences of numbers",
             [] (RandomEngine::Seed seed) {
                 RandomEngine r1(seed);
                 RandomEngine r2(seed);
                 for (int i = 0; i < ENOUGH; i++)
                     RC_ASSERT(r1.nextAtom() == r2.nextAtom());
             });

        prop("a copy of a randomEngine yields the same sequences of numbers as"
             " the original",
             [] (RandomEngine::Seed seed) {
                 RandomEngine r1(seed);
                 RandomEngine r2(r1);
                 for (int i = 0; i < ENOUGH; i++)
                     RC_ASSERT(r1.nextAtom() == r2.nextAtom());
             });

        prop("different seeds yields different sequences of numbers",
             [] {
                 auto s1 = *gen::arbitrary<RandomEngine::Seed>();
                 auto s2 = *gen::suchThat(
                     gen::arbitrary<RandomEngine::Seed>(),
                     [=](const RandomEngine::Seed &s){ return s != s1; });

                 RandomEngine r1(s1);
                 RandomEngine r2(s2);
                 std::vector<RandomEngine::Atom> atoms1;
                 std::vector<RandomEngine::Atom> atoms2;
                 for (int i = 0; i < ENOUGH; i++) {
                     atoms1.push_back(r1.nextAtom());
                     atoms2.push_back(r2.nextAtom());
                 }

                 RC_ASSERT(atoms1 != atoms2);
             });
    }
}
