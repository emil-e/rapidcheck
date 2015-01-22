#pragma once

#include "Any.h"
#include "RandomEngine.h"

namespace rc {
namespace detail {

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
    //! the direct children. If this node has been shrunk, this method returns
    //! an empty array since theey no longer make up the value of the
    //! root.
    std::vector<ValueDescription> example(const gen::Generator<Any> &generator);

    //! Returns the parent node or `nullptr` if there is none.
    const RoseNode *parent() const;

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

    class Observer;
    class ExampleObserver;
    struct CurrentObserver {
        typedef Observer *ValueType;
        static Observer *defaultValue() { return nullptr; }
    };

    // TODO maybe name tryShrink instead
    Any nextShrinkChildren(const gen::Generator<Any> &generator,
                           bool &didShrink);
    Any nextShrinkSelf(const gen::Generator<Any> &generator,
                       bool &didShrink);
    Any generate(const gen::Generator<Any> &generator);
    bool isChildrenExhausted() const;
    void adoptChildren();
    std::string debugDescription() const;
    std::string debugPath() const;
    std::string debugIndexPath() const;
    int depth() const;
    int index() const;

    RoseNode *m_parent;
    // TODO use unique_ptr instead?
    std::vector<RoseNode> m_children;
    std::size_t m_nextChild = 0;
    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;

    std::size_t m_shrinkChild = 0;
    shrink::IteratorUP<Any> m_shrinkIterator;
    Any m_currentValue;
    Any m_acceptedValue;
};

} // namespace detail
} // namespace rc
