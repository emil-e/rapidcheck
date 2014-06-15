#include <catch.hpp>
#include <rapidcheck.h>

using namespace rc;

template<typename Ret, typename Callable>
Ret cleanRoom(const Callable &callable)
{
    // TODO something to make a clean environment for testing
}

TEST_CASE("gen::suchThat") {
    prop("never generates values not satisfying the predicate",
         [] (int max)
         {
             int x = pick(gen::suchThat(gen::arbitrary<int>(),
                                        [=](int x){ return x < max; }));
             return x < max;
         });
}
