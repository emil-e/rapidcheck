#pragma once

#include "rapidcheck/seq/Transform.h"

namespace rc {
namespace detail {

template<typename T>
ErasedGenerator<T>::ErasedGenerator(const gen::Generator<T> *generator)
    : m_generator(generator) {}

template<typename T>
Any ErasedGenerator<T>::generate() const
{
    return Any::of(m_generator->generate());
}

template<typename T>
Seq<Any> ErasedGenerator<T>::shrink(Any value) const
{
    return seq::map(
        m_generator->shrink(std::move(value.get<T>())),
        [](T &&x) { return Any::of(std::move(x)); });
}

} // namespace detail
} // namespace rc
