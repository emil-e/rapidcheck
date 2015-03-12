#pragma once

#include "rapidcheck/Gen.h"
#include "rapidcheck/shrinkable/Create.h"

namespace rc {
namespace test {

// Generator which returns the passed size
Gen<int> genSize() {
    return [](const Random &random, int size) {
        return shrinkable::just(size);
    };
};

// Generator which returns the passed random.
Gen<Random> genRandom() {
    return [](const Random &random, int size) {
        return shrinkable::just(random);
    };
};

} // namespace test
} // namespace rc
