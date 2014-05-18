#pragma once

#include <sstream>

#include "rapidcheck/Show.hpp"

#include "ImplicitParam.hpp"
#include "RandomEngine.hpp"
#include "ValueProxy.hpp"

namespace rc {
namespace detail {

//! Represents the structure of value generation where large complex values are
//! generated from small simple values. This also means that large values often
//! can be shrunk by shrinking the small values individually.
class RoseNode
{
public:
    //! Constructs a new root \c RoseNode.
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

    //! Outputs a string representation of this node and all its children.
    void print(std::ostream &os)
    {
        for (int i = 0; i < depth(); i++)
            os << "  ";
        os << "- " << description() << std::endl;

        for (auto &child : m_children)
            child.print(os);
    }

    //! Generates a value in this node using the given generator.
    template<typename Gen>
    typename Gen::GeneratedType generate(const Gen &generator)
    {
        typedef typename Gen::GeneratedType T;

        // TODO it would be nice if we could pass Generator<T> values here to
        // avoid template instantiation
        ImplicitParam<ShrunkNode> shrunkNode;

        if (!m_frozen) {
            m_valueProxy = ValueProxyUP(
                new TypedValueProxy<T>(generator));
        }

        if (shrunkNode.hasBinding() && (*shrunkNode == nullptr)) {
            m_frozen = true;
            if (callInNode([this]{ return m_valueProxy->nextShrink(); }))
                *shrunkNode = this;
        }

        return regenerate<typename Gen::GeneratedType>();
    }

    //! Regenerates a value using the last generator.
    template<typename T>
    T regenerate()
    {
        return callInNode(dynamic_cast<TypedValueProxy<T> &>(*m_valueProxy));
    }

    //! Picks a value using the given generator in the context of the current
    //! node.
    template<typename Gen>
    typename Gen::GeneratedType pick(const Gen &generator)
    {
        ImplicitParam<NextChildIndex> nextChildIndex;
        if (*nextChildIndex >= m_children.size())
            m_children.push_back(RoseNode(this));
        (*nextChildIndex)++;
        return m_children[*nextChildIndex - 1].generate(generator);
    }

    //! Returns a description of the last generated value.
    ValueDescription describeLast()
    {
        return callInNode([this]{ return m_valueProxy->describe(); });
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

    //! Tries to find an immediate shrink that yields \c false for the given
    //! generator.
    //!
    //! @param value  The value that determines a sucessful shrink.
    //!
    //! @return  A tuple where the first value tells whether the shrinking was
    //!          successful and the second how many shrinks were tried,
    //!          regardless of success.
    template<typename Gen>
    std::tuple<bool, int> shrink(const Gen &generator)
    {
        int numTries = 0;
        while (true) {
            ImplicitParam<ShrunkNode> shrunkNode;
            shrunkNode.let(nullptr);

            bool failed = generate(generator);
            if (*shrunkNode == nullptr)
                return std::make_tuple(false, numTries);

            numTries++;
            if (!failed) {
                (*shrunkNode)->acceptShrink();
                return std::make_tuple(true, numTries);
            }
        }
    }

    //! Accepts the current shrink value restarting the shrinking with that
    //! value.
    void acceptShrink()
    {
        callInNode([this]{ m_valueProxy->acceptShrink(); });
    }

    //! Move constructor.
    RoseNode(RoseNode &&other)
        : m_parent(other.m_parent)
        , m_children(std::move(other.m_children))
        , m_hasAtom(other.m_hasAtom)
        , m_atom(other.m_atom)
        , m_valueProxy(std::move(other.m_valueProxy))
        , m_frozen(other.m_frozen)
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
        m_valueProxy = std::move(rhs.m_valueProxy);
        m_frozen = rhs.m_frozen;
        adoptChildren();
        return *this;
    }

    //! Returns a reference to the current node.
    static RoseNode &current()
    { return **ImplicitParam<CurrentNode>(); }

private:
    RC_DISABLE_COPY(RoseNode)

    // Implicit parameters, see ImplicitParam

    struct CurrentNode { typedef RoseNode *ValueType; };
    struct NextChildIndex { typedef size_t ValueType; };
    struct ShrunkNode { typedef RoseNode *ValueType; };

    //! Constructs a new \c RoseNode with the given parent or \c 0 if it should
    //! have no parent, i.e. is root.
    explicit RoseNode(RoseNode *parent) : m_parent(parent) {}

    //! Returns the depth of this node.
    int depth() const
    {
        if (m_parent == nullptr)
            return 0;

        return m_parent->depth() + 1;
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

    //! Sets the parent of all children to this node.
    void adoptChildren()
    {
        for (auto &child : m_children)
            child.m_parent = this;
    }

    template<typename T>
    TypedValueProxy<T> &typedLastProxy()
    { return dynamic_cast<TypedValueProxy<T> &>(*m_valueProxy); }

    //! Returns a description of this node.
    std::string description() const
    {
        if (!m_valueProxy)
            return "?";

        auto desc(m_valueProxy->typeName());
        if (m_parent != nullptr)
            desc += "[" + std::to_string(index()) + "]";
        return desc;
    }

    //! Returns the index of this node among its sibilings. Returns \c -1 if
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
            return "/ " + description();
        else
            return m_parent->path() + " / " + description();
    }

    typedef std::vector<RoseNode> Children;
    RoseNode *m_parent;
    Children m_children;
    bool m_hasAtom = false;
    RandomEngine::Atom m_atom;
    ValueProxyUP m_valueProxy;
    bool m_frozen = false;
};

} // namespace detail
} // namespace rc
