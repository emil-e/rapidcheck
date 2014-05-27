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

    TestParameters params;
    RandomEngine seedEngine;

    auto property(anyInvocation(testable));
    size_t currentSize = 0;
    for (int testIndex = 1; testIndex <= params.maxSuccess; testIndex++) {
        RandomEngine::Atom seed = seedEngine.nextAtom();
        ImplicitParam<param::RandomEngine> randomEngine;
        randomEngine.let(RandomEngine());
        randomEngine->seed(seed);

        ImplicitParam<param::Size> size;
        size.let(currentSize);

        ImplicitParam<param::NoShrink> noShrink;
        noShrink.let(false);

        if (!property()) {
            std::cout << "...Failed!" << std::endl;
            std::cout << "Shrinking..." << std::flush;
            RoseNode rootNode;
            randomEngine->seed(seed);
            int numShrinks = doShrink(rootNode, property);
            std::cout << std::endl;
            std::cout << "Falsifiable, after " << testIndex
                      << " tests and " << numShrinks << " shrinks:" << std::endl;
            printExample(rootNode);
            return false;
        }

        std::cout << "\r" << testIndex << "/" << params.maxSuccess << std::flush;
        currentSize = std::min(params.maxSize, currentSize + 1);
    }

    std::cout << std::endl << "OK, passed " << params.maxSuccess
              << " tests" << std::endl;
    return true;
}

}
