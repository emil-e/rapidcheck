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

    return m_root.currentValue(
        detail::ErasedGenerator<T>(generator)).template get<T>();
}

template<typename T>
T Rose<T>::nextShrink(const gen::Generator<T> &generator, bool &didShrink)
{
    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(&m_randomEngine);
    ImplicitParam<param::Size> size;
    size.let(m_testCase.size);

    return m_root.nextShrink(
        detail::ErasedGenerator<T>(generator),
        didShrink).template get<T>();
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
