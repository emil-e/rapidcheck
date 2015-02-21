#pragma once

#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/detail/GenerationParams.h"
#include "rapidcheck/detail/RoseNode.h"

namespace rc {
namespace gen {

//! Generates a value.
template<typename T>
T Generator<T>::operator*() const
{
    auto currentNode = detail::ImplicitParam<detail::param::CurrentNode>::value();
    if (currentNode != nullptr) {
        return currentNode->pick(*this);
    } else {
        return generate();
    }
}

template<typename T>
Seq<T> Generator<T>::shrink(T value) const
{
    return Seq<T>();
}

template<typename T>
void sample(int sz, const Generator<T> &generator, uint64_t seed)
{
    using namespace detail;

    ImplicitParam<param::Size> size(sz);

    RandomEngine engine(seed);
    ImplicitParam<param::RandomEngine> randomEngine(&engine);

    show(*generator, std::cout);
    std::cout << std::endl;
}

} // namespace gen
} // namespace rc
