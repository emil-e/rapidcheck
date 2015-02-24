#pragma once

#include "Any.h"
#include "RandomEngine.h"
#include "ErasedGenerator.h"

namespace rc {
namespace detail {

template<typename T> class ErasedGenerator;

//! Internal class used by `Rose`.
class RoseNode
{
    template<typename T> friend class ::rc::gen::Generator;

public:
    RoseNode(RoseNode *parent = nullptr);

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

    //! Returns a vector of `ValueDescription`s describing the current values of
    //! the direct children. If this node has been shrunk, this method returns
    //! an empty array since theey no longer make up the value of the
    //! root.
    std::vector<std::pair<std::string, std::string>>
    example(const gen::Generator<Any> &generator);

    //! Returns the parent node or `nullptr` if there is none.
    const RoseNode *parent() const;

    //! Move constructor.
    RoseNode(RoseNode &&other) noexcept;

    //! Move assignment
    RoseNode &operator=(RoseNode &&rhs) noexcept;

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

    Any pick(const gen::Generator<Any> &generator);

    template<typename T>
    T pick(const gen::Generator<T> &generator)
    {
        return std::move(
            pick(ErasedGenerator<T>(&generator)).template get<T>());
    }


    // TODO maybe name tryShrink instead
    Any nextShrinkChildren(const gen::Generator<Any> &generator,
                           bool &didShrink);
    Any nextShrinkSelf(const gen::Generator<Any> &generator,
                       bool &didShrink);
    Any generate(const gen::Generator<Any> &generator);
    bool isChildrenExhausted() const;
    void adoptChildren();

    RoseNode *m_parent;
    // TODO use unique_ptr instead?
    std::vector<RoseNode> m_children;
    std::size_t m_nextChild = 0;

    std::size_t m_shrinkChild = 0;
    Maybe<Seq<Any>> m_shrinks;
    Any m_currentValue;
    Any m_acceptedValue;
};

} // namespace detail
} // namespace rc
