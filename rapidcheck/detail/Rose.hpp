#pragma once

#include <sstream>

#include "rapidcheck/Show.h"
#include "rapidcheck/Generator.h"

#include "ImplicitParam.hpp"
#include "RandomEngine.hpp"
#include "GenerationParams.hpp"

namespace rc {
namespace detail {

//! Thrown to indicate that the requested type was not expected. This can only
//! happen if generation if what type is requested depends on non-deterministic
//! factors other than what values were generated before.
class UnexpectedTypeException : public std::runtime_error
{
public:
    UnexpectedTypeException(const std::type_info &expected,
                            const std::type_info &actual)
        : std::runtime_error(
            "Expected '" + demangle(expected.name())
            + "' but '" + demangle(actual.name()) + "' was requested")
        , m_expected(expected)
        , m_actual(actual)
    {
    }

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
    RoseNode() : RoseNode(nullptr) {}

    //! Returns an atom. If one has already been generated, it's reused. If not,
    //! a new one is generated.
    RandomEngine::Atom atom()
    {
        if (!m_hasAtom) {
            ImplicitParam<param::RandomEngine> randomEngine;
            m_atom = randomEngine->nextAtom();
            m_hasAtom = true;
        }

        return m_atom;
    }

    //! Outputs the tree structure to the given output stream for debugging.
    void print(std::ostream &os)
    {
        for (int i = 0; i < depth(); i++)
            os << "  ";
        os << "- " << description() << std::endl;

        for (auto &child : m_children)
            child.print(os);
    }

    //! Picks a value using the given generator in the context of the current
    //! node.
    template<typename T>
    T pick(gen::GeneratorUP<T> &&generator)
    {
        ImplicitParam<NextChildIndex> nextChildIndex;
        if (*nextChildIndex >= m_children.size())
            m_children.push_back(RoseNode(this));
        (*nextChildIndex)++;
        return m_children[*nextChildIndex - 1].generate(std::move(generator));
    }

    //! Returns a list of `ValueDescription`s from the immediate children of
    //! this node.
    std::vector<gen::ValueDescription> example()
    {
        // TODO we should capture values during generation instead, somehow...
        std::vector<gen::ValueDescription> values;
        values.reserve(m_children.size());
        for (auto &child : m_children)
            values.push_back(child.regenerateDescription());
        return values;
    }

    //! Regenerates a string representation of the value of this node or an
    //! empty if one hasn't been decided.
    gen::ValueDescription regenerateDescription()
    {
        ImplicitParam<CurrentNode> currentNode;
        currentNode.let(this);
        ImplicitParam<NextChildIndex> nextChildIndex;
        nextChildIndex.let(0);

        gen::UntypedGenerator *generator = activeGenerator();
        if (generator != nullptr)
            return generator->generateDescription();
        else
            return gen::ValueDescription();
    }

    //! Tries to find an immediate shrink that yields the given value.
    //!
    //! @return  A tuple where the first value tells whether the shrinking was
    //!          successful and the second how many shrinks were tried,
    //!          regardless of success.
    template<typename T>
    int shrink(const gen::Generator<T> &generator)
    {
        ImplicitParam<ShrunkNode> shrunkNode;
        int numShrinks = 0;
        T desiredValue(generateWith(generator));
        while (true) {
            shrunkNode.let(nullptr);
            T value(generateWith(generator));
            if (*shrunkNode == nullptr) {
                return numShrinks;
            } else if (value == desiredValue) {
                (*shrunkNode)->acceptShrink();
                numShrinks++;
            }
        }
    }

    //! Returns true if this node is frozen. If a node is frozen, the generator
    //! passed to `generate` will only be used to infer the type, the actual
    //! value will come from shrinking or similar.
    bool isFrozen() const
    {
        return bool(m_acceptedGenerator);
    }

    //! Move constructor.
    RoseNode(RoseNode &&other)
        : m_parent(other.m_parent)
        , m_children(std::move(other.m_children))
        , m_hasAtom(other.m_hasAtom)
        , m_atom(other.m_atom)
        , m_shrinkIterator(std::move(other.m_shrinkIterator))
        , m_originalGenerator(std::move(other.m_originalGenerator))
        , m_acceptedGenerator(std::move(other.m_acceptedGenerator))
        , m_currentGenerator(std::move(other.m_currentGenerator))
    {
        adoptChildren();
    }

    //! Move assignment
    RoseNode &operator=(RoseNode &&rhs)
    {
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
        m_hasAtom = rhs.m_hasAtom;
        m_atom = rhs.m_atom;
        m_shrinkIterator = std::move(rhs.m_shrinkIterator);
        m_originalGenerator = std::move(rhs.m_originalGenerator);
        m_acceptedGenerator = std::move(rhs.m_acceptedGenerator);
        m_currentGenerator = std::move(rhs.m_currentGenerator);
        adoptChildren();
        return *this;
    }

    //! Returns a reference to the current node.
    static RoseNode &current()
    { return **ImplicitParam<CurrentNode>(); }

    //! Returns a reference to the current node.
    static bool hasCurrent()
    { return ImplicitParam<CurrentNode>().hasBinding(); }

private:
    RC_DISABLE_COPY(RoseNode)

    // Implicit parameters, see ImplicitParam
    struct CurrentNode { typedef RoseNode *ValueType; };
    struct NextChildIndex { typedef size_t ValueType; };
    struct ShrunkNode { typedef RoseNode *ValueType; };

    //! Constructs a new `RoseNode` with the given parent or `0` if it should
    //! have no parent, i.e. is root.
    explicit RoseNode(RoseNode *parent) : m_parent(parent) {}

    //! Returns the depth of this node.
    int depth() const
    {
        if (m_parent == nullptr)
            return 0;

        return m_parent->depth() + 1;
    }

    //! Sets the parent of all children to this node.
    void adoptChildren()
    {
        for (auto &child : m_children)
            child.m_parent = this;
    }

    //! Returns a description of this node.
    std::string description() const
    {
        std::string desc(generatorName());
        if (m_parent != nullptr)
            desc += "[" + std::to_string(index()) + "]";
        return desc;
    }

    //! Returns the index of this node among its sibilings. Returns `-1` if
    //! node is root.
    std::ptrdiff_t index() const
    {
        if (m_parent == nullptr)
            return -1;

        auto &siblings = m_parent->m_children;
        auto it = std::find_if(
            siblings.begin(),
            siblings.end(),
            [this](const RoseNode &node){ return &node == this; });

        return it - siblings.begin();
    }

    //! Returns a string describing the path to this node from the root node.
    std::string path()
    {
        if (m_parent == nullptr)
            return "/ " + description() + "\n";
        else
            return m_parent->path() + "/ " + description() + "\n";
    }

    //! Returns the active generator.
    gen::UntypedGenerator *activeGenerator() const
    {
        if (m_currentGenerator)
            return m_currentGenerator.get();
        else if (m_acceptedGenerator)
            return m_acceptedGenerator.get();
        else if (m_originalGenerator)
            return m_originalGenerator.get();
        else
            return nullptr;
    }

    //! Returns the name of the active generator
    std::string generatorName() const
    {
        auto gen = activeGenerator();
        if (gen == nullptr)
            return std::string();
        else
            return demangle(typeid(*gen).name());
    }

    //! Returns the active generator cast to a generator of the given type or
    //! `default` if there is none or if there is a type mismatch.
    template<typename T>
    T regenerate()
    {
        return generateWith(*generatorCast<T>(activeGenerator()));
    }

    //! Accepts the current shrink value
    void acceptShrink()
    {
        if (!m_currentGenerator)
            return;
        m_acceptedGenerator = std::move(m_currentGenerator);
        m_shrinkIterator = nullptr;
    }

    //! Generates a value in this node using the given generator.
    template<typename T>
    T generate(gen::GeneratorUP<T> &&generator)
    {
        return doGenerate(std::move(generator), std::is_copy_constructible<T>());
    }

    template<typename T>
    T doGenerate(gen::GeneratorUP<T> &&generator, std::true_type)
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
    T doGenerate(gen::GeneratorUP<T> &&generator, std::false_type)
    {
        m_originalGenerator = std::move(generator);
        return regenerate<T>();
    }

    template<typename T>
    T generateWith(const gen::Generator<T> &generator)
    {
        ImplicitParam<CurrentNode> currentNode;
        currentNode.let(this);
        ImplicitParam<NextChildIndex> nextChildIndex;
        nextChildIndex.let(0);
        return generator();
    }

    template<typename T>
    static const gen::Generator<T> *generatorCast(
        const gen::UntypedGenerator *gen)
    {
        const auto *typed = dynamic_cast<const gen::Generator<T> *>(gen);
        if (typed == nullptr) {
            throw UnexpectedTypeException(
                typeid(T),
                gen->generatedTypeInfo());
        }

        return typed;
    }

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
