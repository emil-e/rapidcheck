#pragma once

#include "ImplicitParam.hpp"

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
    explicit RoseNode(RoseNode *parent) : m_parent(parent) {}

    //! Returns the number of children.
    size_t childCount()
    { return m_children.size(); }

    //! Returns a reference to the child at the given index. If the index is
    //! outside the bounds, new children will be created up to the given index.
    RoseNode &operator[](size_t index)
    {
        while (index >= m_children.size())
            newChild();
        return m_children[index];
    }

    //! Creates a new child node and returns its index.
    size_t newChild()
    {
        m_children.emplace_back(this);
        return m_children.size() - 1;
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

    //! Prints this node and all its children.
    void print(std::ostream &os) const
    {
        for (int i = 0; i < depth(); i++)
            os << " ";
        os << "***";
        if (m_hasAtom)
            os << " " << m_atom;
        os << std::endl;

        for (const auto &child : m_children)
            child.print(os);
    }

    //! Calls the given callable in the context of this node.
    template<typename Callable, typename ...Args>
    typename std::result_of<Callable(Args...)>::type
    call(const Callable &callable, Args... args)
    {
        ImplicitParam<CurrentNode> currentNode;
        ImplicitParam<NextChildIndex> nextChildIndex;
        currentNode.let(this);
        nextChildIndex.let(0);
        return callable(args...);
    }

    //! Picks a value using the given generator in the context of the current
    //! node.
    template<typename T>
    static T pick(const Generator<T> &generator)
    {
        ImplicitParam<CurrentNode> currentNode;
        ImplicitParam<NextChildIndex> nextChildIndex;
        (*nextChildIndex)++;
        return (**currentNode)[*nextChildIndex - 1].call(generator);
    }

    //! Move constructor.
    RoseNode(RoseNode &&other)
        : m_parent(other.m_parent)
        , m_children(std::move(other.m_children))
        , m_hasAtom(other.m_hasAtom)
        , m_atom(other.m_atom)
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
        adoptChildren();
        return *this;
    }

    //! Returns a reference to the current node.
    static RoseNode &current()
    {
        return **ImplicitParam<CurrentNode>();
    }

private:
    RC_DISABLE_COPY(RoseNode)

    // Implicit parameters
    struct CurrentNode { typedef RoseNode *ValueType; };
    struct NextChildIndex { typedef size_t ValueType; };

    void adoptChildren()
    {
        for (auto &child : m_children)
            child.m_parent = this;
    }

    typedef std::vector<RoseNode> Children;

    RoseNode *m_parent = 0;
    Children m_children;
    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;
};

} // namespace detail
} // namespace rc
