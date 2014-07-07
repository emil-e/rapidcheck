#pragma once

#include "RandomEngine.h"
#include "GeneratorFwd.h"
#include "Utility.h"
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

//! Represents the structure of value generation where large complex values are
//! generated from small simple values. This also means that large values often
//! can be shrunk by shrinking the small values individually.
class RoseNode
{
public:
    RoseNode(RoseNode *parent = nullptr);

    //! Picks a value using the given generator in the context of this
    //! `RoseNode`.
    template<typename T>
    T pick(gen::GeneratorUP<T> &&generator);

    //! Generates a value in the context of this node using the given generator.
    template<typename T>
    void setGenerator(gen::GeneratorUP<T> &&generator);

    //! Returns the current value which may be be generated or fixed.
    template<typename T>
    T currentValue();

    //! Returns the current value which may be be generated or fixed.
    gen::ValueDescription currentDescription();

    //! Returns the next shrink of this `RoseNode`.
    //!
    //! @param didShrink  Set to `true` if there was another shrink or `false`
    //!                   if exhausted.
    //!
    //! @return  The shrunk value.
    template<typename T>
    T nextShrink(bool &didShrink);

    //! Accepts the current shrink.
    void acceptShrink();

    //! Returns an atom. If one has already been generated, it's reused. If not,
    //! a new one is generated.
    RandomEngine::Atom atom();

    //! Returns a vector of `ValueDescription`s describing the current values of
    //! the direct children.
    std::vector<gen::ValueDescription> example();

    //! Move constructor.
    RoseNode(RoseNode &&other);

    //! Move assignment
    RoseNode &operator=(RoseNode &&rhs);

private:
    RC_DISABLE_COPY(RoseNode)

    template<typename T>
    T nextShrink(bool &didShrink, std::true_type);

    template<typename T>
    T nextShrink(bool &didShrink, std::false_type);

    template<typename T>
    T nextShrinkChildren(bool &didShrink);

    gen::UntypedGenerator *currentGenerator() const;

    template<typename T>
    static gen::Generator<T> *generatorCast(gen::UntypedGenerator *gen);

    template<typename T>
    static shrink::Iterator<T> *iteratorCast(
        const shrink::UntypedIteratorUP &it);

    void adoptChildren();

    std::string debugDescription() const;
    std::string debugPath() const;
    std::string debugIndexPath() const;
    int depth() const;
    int index() const;

    RoseNode *m_parent;
    std::vector<RoseNode> m_children;
    std::size_t m_nextChild = 0;
    std::size_t m_shrinkChildren = 0;

    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;
    shrink::UntypedIteratorUP m_shrinkIterator;
    gen::UntypedGeneratorUP m_canonicalGenerator;
    gen::UntypedGeneratorUP m_currentGenerator;
    gen::UntypedGeneratorUP m_acceptedGenerator;
};

} // namespace detail
} // namespace rc

#include "Rose.hpp"
