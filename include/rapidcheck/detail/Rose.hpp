#pragma once

template<typename T>
void doShow(const T &value, std::ostream &os)
{
    using namespace rc;
    show(value, os);
}

namespace rc {
namespace detail {

template<typename T>
Rose<T>::Rose(const gen::Generator<T> *generator, const TestCase &testCase)
    : m_generator(generator)
    , m_testCase(testCase)
    , m_randomEngine(testCase.seed)
{
    // Initialize the tree with the test case.
    currentValue();
}

template<typename T>
T Rose<T>::currentValue()
{
    ImplicitParam<param::RandomEngine> randomEngine(&m_randomEngine);
    ImplicitParam<param::Size> size(m_testCase.size);

    return m_root.currentValue(m_generator).template get<T>();
}

template<typename T>
T Rose<T>::nextShrink(bool &didShrink)
{
    ImplicitParam<param::RandomEngine> randomEngine(&m_randomEngine);
    ImplicitParam<param::Size> size(m_testCase.size);

    return m_root.nextShrink(m_generator, didShrink).template get<T>();
}

template<typename T>
void Rose<T>::acceptShrink()
{
    return m_root.acceptShrink();
}

template<typename T>
std::vector<ValueDescription> Rose<T>::example()
{
    ImplicitParam<param::RandomEngine> randomEngine(&m_randomEngine);
    ImplicitParam<param::Size> size(m_testCase.size);

    return m_root.example(m_generator);
}

} // namespace detail
} // namespace rc
