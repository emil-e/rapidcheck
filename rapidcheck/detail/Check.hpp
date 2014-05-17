#pragma once

#include "Generator.hpp"
#include "RandomEngine.hpp"
#include "ImplicitParam.hpp"
#include "Rose.hpp"

namespace rc {

template<typename Testable>
bool check(Testable testable)
{
    using namespace detail;
    TestParameters params;

    auto property(anyInvocation(testable));
    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(RandomEngine());
    size_t currentSize = 0;
    for (int testIndex = 1; testIndex <= params.maxSuccess; testIndex++) {
        RoseNode rootNode;
        ImplicitParam<param::Size> size;
        size.let(currentSize);

        if (!rootNode.generate(property)) {
            // Test failed!
            for (const auto &desc : rootNode.example()) {
                std::cout << desc.value() << " :: " << desc.typeName() << std::endl;
            }
            return false;
        }

        // TODO do size iteration like Haskells quickcheck
        currentSize = std::min(params.maxSize, currentSize + 1);
    }

    return true;
}

}
