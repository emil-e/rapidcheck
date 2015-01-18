#pragma once

#include "rapidcheck/detail/RandomEngine.h"
#include "rapidcheck/detail/GeneratorFwd.h"
#include "rapidcheck/detail/Utility.h"
#include "rapidcheck/detail/Results.h"
#include "rapidcheck/detail/ValueDescription.h"
#include "rapidcheck/detail/Any.h"
#include "rapidcheck/Shrink.h"

namespace rc {
namespace detail {

//! Thrown to indicate that the requested type was not expected. This can only
//! happen if generation if what type is requested depends on non-deterministic
//! factors other than what values were generated before.
class UnexpectedType : public std::runtime_error
{
public:
    UnexpectedType(const std::type_info &expected,
                   const std::type_info &actual);

private:
    const std::type_info &m_expected;
    const std::type_info &m_actual;
};

//! Internal class used by `Rose`.
class RoseNode
{
public:
    RoseNode(RoseNode *parent = nullptr);

    //! Picks a value using the given generator in the context of this
    //! `RoseNode`.
    Any pick(const gen::Generator<Any> &generator);

    //! Returns the current value which may be be generated or fixed.
    //!
    //! @param generator  The generator in use.
    Any currentValue(const gen::Generator<Any> &generator);

    //! Returns the current value which may be be generated or fixed.
    ValueDescription currentDescription(const gen::UntypedGenerator &generator);

    //! Returns the next shrink of this `RoseNode`. This is the result of
    //! doing a single single shrink operation on a single descendant.
    //!
    //! @param generator  The generator in use.
    //! @param didShrink  Set to `true` if there was another shrink or `false`
    //!                   if exhausted.
    //!
    //! @return  The shrunk value.
    Any nextShrink(const gen::Generator<Any> &generator, bool &didShrink);

    //! Accepts the current shrink.
    void acceptShrink();

    //! Returns an atom. If one has already been generated, it's reused. If not,
    //! a new one is generated.
    RandomEngine::Atom atom();

    //! Returns a vector of `ValueDescription`s describing the current values of
    //! the direct children.
    std::vector<ValueDescription> example(
        const gen::UntypedGenerator &generator);

    //! Move constructor.
    RoseNode(RoseNode &&other);

    //! Move assignment
    RoseNode &operator=(RoseNode &&rhs);

private:
    RC_DISABLE_COPY(RoseNode)

    struct ShrinkMode {
        typedef int ValueType;
        static int defaultValue() { return 0; }
    };

    Any nextShrinkSelf(const gen::Generator<Any> &generator,
                       bool &didShrink);
    Any nextShrinkChildren(const gen::Generator<Any> &generator,
                           bool &didShrink);
    Any generate(const gen::Generator<Any> &generator);
    void adoptChildren();
    std::string debugDescription() const;
    std::string debugPath() const;
    std::string debugIndexPath() const;
    int depth() const;
    int index() const;

    RoseNode *m_parent;
    std::vector<RoseNode> m_children;
    std::size_t m_nextChild = 0;
    std::size_t m_shrinkChild = 0;

    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;
    shrink::IteratorUP<Any> m_shrinkIterator;
    Any m_currentValue;
    Any m_acceptedValue;
};

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
    Rose(const gen::Generator<T> &generator, const TestCase &testCase);

    //! Returns the current value.
    T currentValue(const gen::Generator<T> &generator);

    //! Returns the next shrink of this `Rose`.
    //!
    //! @param didShrink  Set to `true` if there was another shrink or `false`
    //!                   if exhausted.
    //!
    //! @return  The shrunk value.
    T nextShrink(const gen::Generator<T> &generator, bool &didShrink);

    //! Accepts the current shrink.
    void acceptShrink();

    //! Returns a vector of `ValueDescription`s describing the current values of
    //! the direct children.
    std::vector<ValueDescription> example(
        const gen::UntypedGenerator &generator);

private:
    RoseNode m_root;
    TestCase m_testCase;
    RandomEngine m_randomEngine;
};

} // namespace detail
} // namespace rc

#include "Rose.hpp"
