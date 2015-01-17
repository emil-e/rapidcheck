#pragma once

template<typename T>
ErasedGenerator::ErasedGenerator(const gen::Generator<T> &generator)
    : m_generator(generator) {}

template<typename T>
Any ErasedGenerator::generate() const { return m_generator.generate(); }

template<typename T>
shrink::IteratorUP<T> ErasedGenerator::shrink(Any value) const
{
    return shrink::map(
        m_generator.shrink(std::move(value.get<T>())),
        [](T &&x) { return x; });
}
