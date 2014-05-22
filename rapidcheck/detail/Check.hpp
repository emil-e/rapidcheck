#pragma once

#include "Generator.hpp"
#include "RandomEngine.hpp"
#include "ImplicitParam.hpp"
#include "Rose.hpp"

namespace rc {

template<typename Gen>
int doShrink(detail::RoseNode &rootNode, const Gen &generator)
{
    int numShrinks = 0;
    while (true) {
        bool didShrink;
        int numTries;
        std::tie(didShrink, numTries) = rootNode.shrink(generator);
        if (!didShrink)
            return numShrinks;

        numShrinks++;
    }
}

void printExample(detail::RoseNode &rootNode)
{
    for (const auto &v : rootNode.example())
        std::cout << v << std::endl;
}

template<typename Testable>
bool check(Testable testable)
{
    using namespace detail;

    auto property(anyInvocation(testable));
    TestParameters params;
    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(RandomEngine());
    size_t currentSize = 0;
    for (int testIndex = 1; testIndex <= params.maxSuccess; testIndex++) {
        RoseNode rootNode;
        ImplicitParam<param::Size> size;
        size.let(currentSize);
        ImplicitParam<param::NoShrink> noShrink;
        noShrink.let(false);

        if (!rootNode.generate(property)) {
            int numShrinks = doShrink(rootNode, property);
            std::cout << "Falsifiable after " << testIndex << " tests and "
                      << numShrinks << " shrinks:" << std::endl;
            printExample(rootNode);
            return false;
        }

        // TODO do size iteration like Haskells quickcheck
        currentSize = std::min(params.maxSize, currentSize + 1);
    }

    return true;
}

}
