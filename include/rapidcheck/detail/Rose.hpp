#pragma once

#include "ImplicitParam.h"
#include "GenerationParams.h"
#include "Traits.h"
#include "rapidcheck/Generator.h"

template<typename T>
void doShow(const T &value, std::ostream &os)
{
    using namespace rc;
    show(value, os);
}

namespace rc {
namespace detail {

template<typename T>
T RoseNode::pick(const gen::Generator<T> &generator)
{
    auto i = m_nextChild;
    if (m_nextChild >= m_children.size())
        m_children.emplace_back(this);
    m_nextChild++;

    auto &child = m_children[i];
    ImplicitParam<ShrinkMode> shrinkMode;
    if (*shrinkMode && (i == m_shrinkChild)) {
        bool didShrink;
        T value(child.nextShrink<T>(generator, didShrink));
        if (!didShrink)
            m_shrinkChild++;
        return std::move(value);
    } else {
        return child.currentValue<T>(generator);
    }
}

template<typename T>
T RoseNode::currentValue(const gen::Generator<T> &generator)
{
    if (m_currentValue) {
        // TODO hack!
        // For non-copyable types we will never get here, but still, hack
        return std::move(m_currentValue.get<T>());
    } else if (m_acceptedValue) {
        // TODO hack!
        return std::move(m_acceptedValue.get<T>());
    } else {
        ImplicitParam<ShrinkMode> shrinkMode;
        shrinkMode.let(false);
        return generate<T>(generator);
    }
}

template<typename T>
T RoseNode::nextShrink(const gen::Generator<T> &generator, bool &didShrink)
{
    ImplicitParam<param::NoShrink> noShrink;
    if (*noShrink) {
        didShrink = false;
        return currentValue<T>(generator);
    } else {
        return nextShrink<T>(generator, didShrink, IsCopyConstructible<T>());
    }
}

// For copy-constructible types
template<typename T>
T RoseNode::nextShrink(const gen::Generator<T> &generator,
                       bool &didShrink,
                       std::true_type)
{
    if (!m_shrinkIterator) {
        if (!m_acceptedValue) {
            // If we don't have a shrink iterator, shrink the children first.
            T value(nextShrinkChildren<T>(generator, didShrink));
            // If any child did shrink, then we shouldn't shrink ourselves just yet.
            if (didShrink)
                return value;

            // TODO this seems funny... what if we already have an accepted value?
            //      This is set unconditionally and we WILL get here if we accept a
            //      value, right?
            // TODO test for this!
            // Otherwise, we should make a shrink iterator
            // The already generated value is a last restort, set as accepted.
            m_acceptedValue = value;
            m_shrinkIterator = shrink::UntypedIteratorUP(
                generator.shrink(std::move(value)));

            // Since the value of every child is now fixed, the number of
            // children won't change so we can get rid of any unused ones.
            m_children.resize(m_nextChild);
        } else {
            m_shrinkIterator = shrink::UntypedIteratorUP(
                generator.shrink(currentValue(generator)));
        }
    }

    auto typedIterator = iteratorCast<T>(m_shrinkIterator);
    if (typedIterator->hasNext()) {
        // Current iterator still has more shrinks, use that.
        didShrink = true;
        auto value = typedIterator->next();
        m_currentValue = value;
        return value;
    } else {
        // Exhausted
        didShrink = false;
        // Replace with shrink::nothing since that is likely smaller than the
        // existing iterator.
        m_shrinkIterator = shrink::nothing<T>();
        m_currentValue.reset();
        return currentValue<T>(generator);
    }
}

// For non-copy-constructible types.
template<typename T>
T RoseNode::nextShrink(const gen::Generator<T> &generator,
                       bool &didShrink,
                       std::false_type)
{
    // Only shrink children, cannot explicitly shrink non-copy-constructible
    // types.
    return nextShrinkChildren<T>(generator, didShrink);
}

// Returns the next shrink by shrinking the children or the currently accepted
// if there are no more possible shrinks of the children. `didShrink` is set to
// true if any of them were shrunk.
template<typename T>
T RoseNode::nextShrinkChildren(const gen::Generator<T> &generator,
                               bool &didShrink)
{
    ImplicitParam<ShrinkMode> shrinkMode;
    shrinkMode.let(true);
    T value(generate<T>(generator));
    didShrink = m_shrinkChild < m_nextChild;
    return value;
}

template<typename T>
T RoseNode::generate(const gen::Generator<T> &generator)
{
    ImplicitParam<param::CurrentNode> currentNode;
    currentNode.let(this);
    m_nextChild = 0;
    return generator.generate();
}

template<typename T>
shrink::Iterator<T> *RoseNode::iteratorCast(
    const shrink::UntypedIteratorUP &it)
{
    shrink::Iterator<T> *typed = dynamic_cast<shrink::Iterator<T> *>(it.get());
    // TODO use typeid of ShrunkTypes, not iterators themselves
    if (typed == nullptr)
        throw UnexpectedType(typeid(shrink::Iterator<T>), typeid(it));
    return typed;
}

template<typename T>
Rose<T>::Rose(const gen::Generator<T> &generator, const TestCase &testCase)
    : m_testCase(testCase)
{
    m_randomEngine.seed(testCase.seed);
    // Initialize the tree with the test case.
    currentValue(generator);
}

template<typename T>
T Rose<T>::currentValue(const gen::Generator<T> &generator)
{
    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(&m_randomEngine);
    ImplicitParam<param::Size> size;
    size.let(m_testCase.size);

    return m_root.currentValue<T>(generator);
}

template<typename T>
T Rose<T>::nextShrink(const gen::Generator<T> &generator, bool &didShrink)
{
    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(&m_randomEngine);
    ImplicitParam<param::Size> size;
    size.let(m_testCase.size);

    return m_root.nextShrink<T>(generator, didShrink);
}

template<typename T>
void Rose<T>::acceptShrink()
{
    return m_root.acceptShrink();
}

template<typename T>
std::vector<ValueDescription> Rose<T>::example(
    const gen::UntypedGenerator &generator)
{
    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(&m_randomEngine);
    ImplicitParam<param::Size> size;
    size.let(m_testCase.size);

    return m_root.example(generator);
}

} // namespace detail
} // namespace rc
