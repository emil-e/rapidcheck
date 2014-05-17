#pragma once

#include <sstream>

#include "rapidcheck/Show.hpp"

#include "ImplicitParam.hpp"
#include "RandomEngine.hpp"
#include "GeneratorProxy.hpp"

namespace rc {
namespace detail {

//! Represents the structure of value generation where large complex values are
//! generated from small simple values. This also means that large values often
//! can be shrunk by shrinking the small values individually.
class RoseNode
{
public:

    //! Constructs a new \c RoseNode with the given parent or \c 0 if it should
    //! have no parent.
    explicit RoseNode(RoseNode *parent = 0) : m_parent(parent) {}

    //! Returns the number of children.
    size_t childCount()
    { return m_children.size(); }

    //! Returns a reference to the child at the given index.
    RoseNode &operator[](size_t index)
    {
        return m_children[index];
    }

    //! Returns the depth of this node.
    int depth() const
    {
        if (m_parent == 0)
            return 0;

        return m_parent->depth() + 1;
    }

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

    //! Outputs a string representation of this node and all its children.
    void print(std::ostream &os) const
    {
        //TODO something nice here...
    }

    //! Calls the given callable in the context of this node.
    template<typename Callable, typename ...Args>
    typename std::result_of<Callable(Args...)>::type
    callInNode(const Callable &callable, Args... args)
    {
        ImplicitParam<CurrentNode> currentNode;
        ImplicitParam<NextChildIndex> nextChildIndex;
        currentNode.let(this);
        nextChildIndex.let(0);
        return callable(args...);
    }

    //! Generates a value in this node using the given generator.
    template<typename Gen>
    typename Gen::GeneratedType generate(const Gen &generator)
    {
        m_lastProxy = GeneratorProxyUP(
            new TypedGeneratorProxy<typename Gen::GeneratedType>(generator));
        return regenerate<typename Gen::GeneratedType>();
    }

    //! Generates a value in this node using the last generator.
    template<typename T>
    T regenerate()
    {
        return callInNode(typedLastProxy<T>());
    }

    //! Picks a value using the given generator in the context of the current
    //! node.
    template<typename Gen>
    typename Gen::GeneratedType pick(const Gen &generator)
    {
        ImplicitParam<NextChildIndex> nextChildIndex;
        if (*nextChildIndex >= childCount())
            m_children.emplace_back();
        (*nextChildIndex)++;
        return (*this)[*nextChildIndex - 1].generate(generator);
    }

    //! Returns a description of the last generated value.
    ValueDescription describeLast()
    {
        return callInNode([this]{ return m_lastProxy->describe(); });
    }

    //! Returns a list of \c ValueDescriptions from the immediate children of
    //! this node.
    std::vector<ValueDescription> example()
    {
        std::vector<ValueDescription> descriptions;
        // TODO need to filter out irrelevant children
        for (auto &child : m_children)
            descriptions.push_back(child.describeLast());
        return descriptions;
    }

    //! Move constructor.
    RoseNode(RoseNode &&other)
        : m_parent(other.m_parent)
        , m_children(std::move(other.m_children))
        , m_hasAtom(other.m_hasAtom)
        , m_atom(other.m_atom)
        , m_lastProxy(std::move(other.m_lastProxy))
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
        m_lastProxy = std::move(rhs.m_lastProxy);
        adoptChildren();
        return *this;
    }

    //! Returns a reference to the current node.
    static RoseNode &current()
    { return **ImplicitParam<CurrentNode>(); }

private:
    RC_DISABLE_COPY(RoseNode)

    // Implicit parameters
    struct CurrentNode { typedef RoseNode *ValueType; };
    struct NextChildIndex { typedef size_t ValueType; };
    struct ShrinkMode { typedef bool ValueType; };

    void adoptChildren()
    {
        for (auto &child : m_children)
            child.m_parent = this;
    }

    template<typename T>
    TypedGeneratorProxy<T> &typedLastProxy()
    { return static_cast<TypedGeneratorProxy<T> &>(*m_lastProxy); }

    typedef std::vector<RoseNode> Children;

    RoseNode *m_parent = 0;
    Children m_children;
    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;
    GeneratorProxyUP m_lastProxy;
};

} // namespace detail
} // namespace rc
