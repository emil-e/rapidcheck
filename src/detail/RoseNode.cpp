#include "rapidcheck/detail/Rose.h"

namespace rc {
namespace detail {

class RoseNode::Observer
{
public:
    virtual void pickedValue(const RoseNode &node, const Any &value) = 0;
};

// Used by example() to capture direct sub values of current node
class RoseNode::ExampleObserver : public RoseNode::Observer
{
public:
    ExampleObserver(const RoseNode *node)
        : m_node(node) {}

    void pickedValue(const RoseNode &node, const Any &value) override
    {
        if (node.parent() == m_node)
            m_descriptions.push_back(value.describe());
    }

    std::vector<std::pair<std::string, std::string>> descriptions()
    { return m_descriptions; }

private:
    std::vector<std::pair<std::string, std::string>> m_descriptions;
    const RoseNode *m_node;
};

RoseNode::RoseNode(RoseNode *parent)
    : m_parent(parent) {}

Any RoseNode::pick(const gen::Generator<Any> &generator)
{
    auto i = m_nextChild;
    if (m_nextChild >= m_children.size())
        m_children.emplace_back(this);
    m_nextChild++;

    auto &child = m_children[i];
    Any value;
    if (ImplicitParam<ShrinkMode>::value() && (i == m_shrinkChild)) {
        bool didShrink;
        value = child.nextShrink(generator, didShrink);
        if (!didShrink)
            m_shrinkChild++;
    } else {
        value = child.currentValue(generator);
    }

    auto currentObserver = ImplicitParam<CurrentObserver>::value();
    if (currentObserver != nullptr)
        currentObserver->pickedValue(child, value);
    return value;
}

Any RoseNode::currentValue(const gen::Generator<Any> &generator)
{
    if (m_currentValue) {
        return m_currentValue;
    } else if (m_acceptedValue) {
        return m_acceptedValue;
    } else {
        ImplicitParam<ShrinkMode> shrinkMode(false);
        return generate(generator);
    }
}

Any RoseNode::nextShrink(const gen::Generator<Any> &generator,
                         bool &didShrink)
{
    // If shrinking is disabled, just return the current value
    if (ImplicitParam<param::NoShrink>::value()) {
        didShrink = false;
        return currentValue(generator);
    }

    if (m_shrinks) {
        // We have a shrink sequence, shrink next self
        return nextShrinkSelf(generator, didShrink);
    }

    if (m_acceptedValue) {
        // We have a previosly accepted value but no shrink sequence, that
        // means we want to start shrinking again from the accepted value.
        m_shrinks = generator.shrink(m_acceptedValue);
        return nextShrinkSelf(generator, didShrink);
    }

    // We don't have a shrink sequence so we want to shrink children
    // first
    bool childShrunk;
    Any value = nextShrinkChildren(generator, childShrunk);

    if (childShrunk) {
        // One of the children shrunk and we only want a single change
        // for every invocation of this method, return immediately with
        // value.
        didShrink = true;
        return value;
    }

    if (!value.isCopyable()) {
        // Value is not copyable so we cannot shrink it.
        // Since the value of every child is now fixed, the number
        // of children won't change so we can get rid of any unused
        // ones.
        m_children.resize(m_nextChild);
        didShrink = false;
        return value;
    }

    // Children did not shrink and the value we got is copyable
    // so we should now shrink this node!
    m_shrinks = generator.shrink(value);
    return nextShrinkSelf(generator, didShrink);
}

Any RoseNode::nextShrinkSelf(const gen::Generator<Any> &generator,
                             bool &didShrink)
{
    Maybe<Any> value = m_shrinks->next();
    if (value) {
        // Current sequence still has more shrinks, use that.
        didShrink = true;
        m_currentValue = std::move(*value);
        return m_currentValue;
    } else {
        // Exhausted
        m_currentValue.reset();
        didShrink = false;
        return currentValue(generator);
    }
}

void RoseNode::acceptShrink()
{
    if (m_shrinks) {
        assert(!!m_currentValue);
        m_acceptedValue = std::move(m_currentValue);

        // Since we now have an accepted value, the children are
        // useless so clear them.
        m_children.clear();

        m_shrinks.reset();
    } else {
        assert(m_shrinkChild < m_children.size());
        m_children[m_shrinkChild].acceptShrink();
    }
}

RandomEngine::Atom RoseNode::atom()
{
    if (!m_hasAtom) {
        m_atom = ImplicitParam<param::RandomEngine>::value()->nextAtom();
        m_hasAtom = true;
    }

    return m_atom;
}

std::vector<std::pair<std::string, std::string>> RoseNode::example(
    const gen::Generator<Any> &generator)
{
    ExampleObserver observer(this);
    ImplicitParam<CurrentObserver> currentObserver(&observer);
    currentValue(generator);
    return std::move(observer.descriptions());
}

const RoseNode *RoseNode::parent() const
{ return m_parent; }

// Returns the next shrink by shrinking the children or the currently accepted
// if there are no more possible shrinks of the children. `didShrink` is set to
// true if any of them were shrunk.>
Any RoseNode::nextShrinkChildren(const gen::Generator<Any> &generator,
                                 bool &didShrink)
{
    if (isChildrenExhausted()) {
        didShrink = false;
        return currentValue(generator);
    }

    ImplicitParam<ShrinkMode> shrinkMode(true);
    Any value(generate(generator));
    didShrink = !isChildrenExhausted();
    return value;
}

bool RoseNode::isChildrenExhausted() const
{
    return m_shrinkChild >= m_nextChild;
}

Any RoseNode::generate(const gen::Generator<Any> &generator)
{
    ImplicitParam<param::CurrentNode> currentNode(this);
    m_nextChild = 0;
    return generator.generate();
}

RoseNode::RoseNode(RoseNode &&other) noexcept
    : m_parent(other.m_parent)
    , m_children(std::move(other.m_children))
    , m_nextChild(other.m_nextChild)
    , m_shrinkChild(other.m_shrinkChild)
    , m_hasAtom(other.m_hasAtom)
    , m_atom(other.m_atom)
    , m_shrinks(std::move(other.m_shrinks))
    , m_currentValue(std::move(other.m_currentValue))
    , m_acceptedValue(std::move(other.m_acceptedValue))
{
    adoptChildren();
}

RoseNode &RoseNode::operator=(RoseNode &&rhs) noexcept
{
    m_parent = rhs.m_parent;
    m_children = std::move(rhs.m_children);
    m_nextChild = rhs.m_nextChild;
    m_shrinkChild = rhs.m_shrinkChild;
    m_hasAtom = rhs.m_hasAtom;
    m_atom = rhs.m_atom;
    m_shrinks = std::move(rhs.m_shrinks);
    m_currentValue = std::move(rhs.m_currentValue);
    m_acceptedValue = std::move(rhs.m_acceptedValue);
    adoptChildren();
    return *this;
}

void RoseNode::adoptChildren()
{
    for (auto &child : m_children)
        child.m_parent = this;
}

} // namespace detail
} // namespace rc
