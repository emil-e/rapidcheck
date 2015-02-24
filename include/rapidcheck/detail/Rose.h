#pragma once

#include "ErasedGenerator.h"
#include "Results.h"

namespace rc {
namespace detail {

//! Used to implement implicit shrinking of the generation tree. Values are
//! implicitly shrunk from the leaves working up to the root.
template<typename T>
class Rose
{
public:
    //! Constructor.
    //!
    //! @param generator  The generator to use.
    //! @param testCase   The test case to use.
    Rose(const gen::Generator<T> *generator, const TestCase &testCase);

    //! See `RoseNode::currentValue`
    T currentValue();

    //! See `RoseNode::nextShrink`
    T nextShrink(bool &didShrink);

    //! See `RoseNode::acceptShrink`
    void acceptShrink();

    //! See `RoseNode::example`
    std::vector<std::pair<std::string, std::string>> example();

private:
    RC_DISABLE_COPY(Rose)

    const ErasedGenerator<T> m_generator;
    RoseNode m_root;
    const TestCase m_testCase;
};

} // namespace detail
} // namespace rc

#include "Rose.hpp"
