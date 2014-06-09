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
    //! Constructs a new root `RoseNode`.
    RoseNode();

    //! Returns an atom. If one has already been generated, it's reused. If not,
    //! a new one is generated.
    RandomEngine::Atom atom();

    //! Outputs the tree structure to the given output stream for debugging.
    void print(std::ostream &os);

    //! Returns a list of `ValueDescription`s from the immediate children of
    //! this node.
    std::vector<gen::ValueDescription> example();

    //! Regenerates a string representation of the value of this node or an
    //! empty if one hasn't been decided.
    gen::ValueDescription regenerateDescription();

    //! Returns true if this node is frozen. If a node is frozen, the generator
    //! passed to `generate` will only be used to infer the type, the actual
    //! value will come from shrinking or similar.
    bool isFrozen() const;

    //! Move constructor.
    RoseNode(RoseNode &&other);

    //! Move assignment
    RoseNode &operator=(RoseNode &&rhs);

    //! Returns a reference to the current node.
    static RoseNode &current();

    //! Returns a reference to the current node.
    static bool hasCurrent();

    //! Picks a value using the given generator in the context of the current
    //! node.
    template<typename T>
    T pick(gen::GeneratorUP<T> &&generator);

    //! Tries to find an immediate shrink that yields the given value.
    //!
    //! @return  A tuple where the first value tells whether the shrinking was
    //!          successful and the second how many shrinks were tried,
    //!          regardless of success.
    template<typename T>
    int shrink(const gen::Generator<T> &generator);

private:
    RC_DISABLE_COPY(RoseNode)

    // Implicit parameters, see ImplicitParam
    struct CurrentNode { typedef RoseNode *ValueType; };
    struct NextChildIndex { typedef size_t ValueType; };
    struct ShrunkNode { typedef RoseNode *ValueType; };

    //! Constructs a new `RoseNode` with the given parent or `0` if it should
    //! have no parent, i.e. is root.
    explicit RoseNode(RoseNode *parent);

    //! Returns the depth of this node.
    int depth() const;

    //! Sets the parent of all children to this node.
    void adoptChildren();

    //! Returns a description of this node.
    std::string description() const;

    //! Returns the index of this node among its sibilings. Returns `-1` if
    //! node is root.
    std::ptrdiff_t index() const;

    //! Returns a string describing the path to this node from the root node.
    std::string path();

    //! Returns the active generator.
    gen::UntypedGenerator *activeGenerator() const;

    //! Returns the name of the active generator
    std::string generatorName() const;

    //! Accepts the current shrink value
    void acceptShrink();

    //! Returns the active generator cast to a generator of the given type or
    //! `default` if there is none or if there is a type mismatch.
    template<typename T>
    T regenerate();

    //! Generates a value in this node using the given generator.
    template<typename T>
    T generate(gen::GeneratorUP<T> &&generator);

    template<typename T>
    T doGenerate(gen::GeneratorUP<T> &&generator, std::true_type);

    template<typename T>
    T doGenerate(gen::GeneratorUP<T> &&generator, std::false_type);

    template<typename T>
    T generateWith(const gen::Generator<T> &generator);

    template<typename T>
    static const gen::Generator<T> *generatorCast(
        const gen::UntypedGenerator *gen);

    typedef std::vector<RoseNode> Children;
    RoseNode *m_parent;
    Children m_children;
    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;
    shrink::UntypedIteratorUP m_shrinkIterator;

    // The generator that was used to generate the original value, before
    // shrinking
    gen::UntypedGeneratorUP m_originalGenerator;
    // When a shrunk value is accepted as a valid shrink, a generator for that
    // value is saved here
    gen::UntypedGeneratorUP m_acceptedGenerator;
    // Any temporary values (e.g. when shrinking) are saved as generators here
    gen::UntypedGeneratorUP m_currentGenerator;
};

} // namespace detail
} // namespace rc

#include "Rose.hpp"
