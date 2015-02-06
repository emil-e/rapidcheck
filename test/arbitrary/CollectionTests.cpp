#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "util/TypeListMacros.h"
#include "util/Meta.h"
#include "util/Util.h"
#include "util/Predictable.h"

using namespace rc;

struct CollectionTests
{
    template<typename T>
    static void exec()
    {
        templatedProp<T>(
            "uses the correct arbitrary instance",
            [] {
                auto values = *gen::arbitrary<T>();
                for (const auto &value : values)
                    RC_ASSERT(isArbitraryPredictable(value));
            });
    }
};

TEST_CASE("gen::arbitrary for containers") {
    meta::forEachType<CollectionTests,
                      RC_GENERIC_CONTAINERS(Predictable),
                      RC_GENERIC_CONTAINERS(NonCopyable),
                      std::array<Predictable, 100>,
                      std::array<NonCopyable, 100>>();
}
