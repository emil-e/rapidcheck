#pragma once

#include "ImplicitParam.h"
#include "GenerationParams.h"
#include "Traits.h"
#include "rapidcheck/Generator.h"

namespace rc {
namespace detail {

template<typename T>
T RoseNode::pick(gen::GeneratorUP<T> &&generator)
{
    ImplicitParam<NextChildIndex> nextChildIndex;
    if (*nextChildIndex >= m_children.size())
        m_children.push_back(RoseNode(this));
    (*nextChildIndex)++;
    return m_children[*nextChildIndex - 1].generate(std::move(generator));
}

template<typename T>
std::tuple<T, int> RoseNode::shrink(const gen::Generator<T> &generator)
{
    ImplicitParam<ShrunkNode> shrunkNode;
    int numShrinks = 0;
    T desiredValue(generateWith(generator));
    while (true) {
        shrunkNode.let(nullptr);
        T value(generateWith(generator));
        if (*shrunkNode == nullptr) {
            return std::make_tuple(value, numShrinks);
        } else if (value == desiredValue) {
            (*shrunkNode)->acceptShrink();
            numShrinks++;
        }
    }
}


template<typename T>
T RoseNode::generateWith(const gen::Generator<T> &generator)
{
    ImplicitParam<param::CurrentNode> currentNode;
    currentNode.let(this);
    ImplicitParam<NextChildIndex> nextChildIndex;
    nextChildIndex.let(0);
    return generator();
}


//! Generates a value in this node using the given generator.
template<typename T>
T RoseNode::generate(gen::GeneratorUP<T> &&generator)
{
    return doGenerate(std::move(generator), IsCopyConstructible<T>());
}

template<typename T>
T RoseNode::doGenerate(gen::GeneratorUP<T> &&generator, std::true_type)
{
    if (!isFrozen())
        m_originalGenerator = std::move(generator);

    ImplicitParam<ShrunkNode> shrunkNode;
    if (shrunkNode.hasBinding() && (*shrunkNode == nullptr)) {
        if (!m_shrinkIterator) {
            T value(regenerate<T>());
            // Did children shrink before us?
            if (*shrunkNode != nullptr)
                return value;

            ImplicitParam<param::NoShrink> noShrink;
            if (*noShrink) {
                // TODO there might be further optimizations here if we can
                // smarter with no-shrink
                m_shrinkIterator = shrink::nothing<T>();
            } else {
                m_shrinkIterator = generatorCast<T>(
                    m_originalGenerator.get())->shrink(std::move(value));
            }
        }

        if (m_shrinkIterator->hasNext()) {
            auto typedIterator =
                dynamic_cast<shrink::Iterator<T> *>(m_shrinkIterator.get());
            assert(typedIterator != nullptr);
            m_currentGenerator = gen::UntypedGeneratorUP(
                new gen::Constant<T>(typedIterator->next()));
            *shrunkNode = this;
        } else {
            // Shrinking exhausted
            m_currentGenerator = nullptr;
        }
    }

    return regenerate<T>();
}

template<typename T>
T RoseNode::doGenerate(gen::GeneratorUP<T> &&generator, std::false_type)
{
    m_originalGenerator = std::move(generator);
    return regenerate<T>();
}


//! Returns the active generator cast to a generator of the given type or
//! `default` if there is none or if there is a type mismatch.
template<typename T>
T RoseNode::regenerate()
{
    return generateWith(*generatorCast<T>(activeGenerator()));
}

template<typename T>
const gen::Generator<T> *RoseNode::generatorCast(
    const gen::UntypedGenerator *gen)
{
    const auto *typed = dynamic_cast<const gen::Generator<T> *>(gen);
    if (typed == nullptr) {
        throw UnexpectedType(
            typeid(T),
            gen->generatedTypeInfo());
    }

    return typed;
}

} // namespace detail
} // namespace rc
