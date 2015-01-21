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

    std::vector<ValueDescription> descriptions() { return m_descriptions; }

private:
    std::vector<ValueDescription> m_descriptions;
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
    ImplicitParam<ShrinkMode> shrinkMode;
    Any value;
    if (*shrinkMode && (i == m_shrinkChild)) {
        bool didShrink;
        value = child.nextShrink(generator, didShrink);
        if (!didShrink)
            m_shrinkChild++;
    } else {
        value = child.currentValue(generator);
    }

    ImplicitParam<CurrentObserver> currentObserver;
    if (*currentObserver != nullptr)
        (*currentObserver)->pickedValue(child, value);
    return value;
}

Any RoseNode::currentValue(const gen::Generator<Any> &generator)
{
    if (m_currentValue) {
        return m_currentValue;
    } else if (m_acceptedValue) {
        return m_acceptedValue;
    } else {
        ImplicitParam<ShrinkMode> shrinkMode;
        shrinkMode.let(false);
        return generate(generator);
    }
}

Any RoseNode::nextShrink(const gen::Generator<Any> &generator,
                         bool &didShrink)
{
    // If shrinking is disabled, just return the current value
    ImplicitParam<param::NoShrink> noShrink;
    if (*noShrink) {
        didShrink = false;
        return currentValue(generator);
    }

    if (m_shrinkIterator) {
        // We have a shrink iterator, shrink next self
        return nextShrinkSelf(generator, didShrink);
    }

    if (m_acceptedValue) {
        // We have a previosly accepted value but no shrink iterator, that
        // means we want to start shrinking again from the accepted value.
        m_shrinkIterator = generator.shrink(m_acceptedValue);
        return nextShrinkSelf(generator, didShrink);
    }

    // We don't have a shrink iterator so we want to shrink children
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
    m_shrinkIterator = generator.shrink(value);
    if (!m_shrinkIterator->hasNext()) {
        // No shrinks for this value, just return the original value
        didShrink = false;
        return value;
    }

    // Otherwise, try to shrink self, finally
    return nextShrinkSelf(generator, didShrink);
}

Any RoseNode::nextShrinkSelf(const gen::Generator<Any> &generator,
                             bool &didShrink)
{
    if (m_shrinkIterator->hasNext()) {
        // Current iterator still has more shrinks, use that.
        didShrink = true;
        m_currentValue = m_shrinkIterator->next();
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
    if (m_shrinkIterator) {
        assert(!!m_currentValue);
        m_acceptedValue = std::move(m_currentValue);

        // Since we now have an accepted value, the children are
        // useless so clear them.
        m_children.clear();

        m_shrinkIterator.reset();
    } else {
        assert(m_shrinkChild < m_children.size());
        m_children[m_shrinkChild].acceptShrink();
    }
}

RandomEngine::Atom RoseNode::atom()
{
    if (!m_hasAtom) {
        ImplicitParam<param::RandomEngine> randomEngine;
        m_atom = (*randomEngine)->nextAtom();
        m_hasAtom = true;
    }

    return m_atom;
}

std::vector<ValueDescription> RoseNode::example(
    const gen::Generator<Any> &generator)
{
    ImplicitParam<CurrentObserver> currentObserver;
    ExampleObserver observer(this);
    currentObserver.let(&observer);
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

    ImplicitParam<ShrinkMode> shrinkMode;
    shrinkMode.let(true);
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
    ImplicitParam<param::CurrentNode> currentNode;
    currentNode.let(this);
    m_nextChild = 0;
    return generator.generate();
}

RoseNode::RoseNode(RoseNode &&other)
    : m_parent(other.m_parent)
    , m_children(std::move(other.m_children))
    , m_nextChild(other.m_nextChild)
    , m_shrinkChild(other.m_shrinkChild)
    , m_hasAtom(other.m_hasAtom)
    , m_atom(other.m_atom)
    , m_shrinkIterator(std::move(other.m_shrinkIterator))
    , m_currentValue(std::move(other.m_currentValue))
    , m_acceptedValue(std::move(other.m_acceptedValue))
{
    adoptChildren();
}

RoseNode &RoseNode::operator=(RoseNode &&rhs)
{
    m_parent = rhs.m_parent;
    m_children = std::move(rhs.m_children);
    m_nextChild = rhs.m_nextChild;
    m_shrinkChild = rhs.m_shrinkChild;
    m_hasAtom = rhs.m_hasAtom;
    m_atom = rhs.m_atom;
    m_shrinkIterator = std::move(rhs.m_shrinkIterator);
    m_currentValue = std::move(rhs.m_currentValue);
    m_acceptedValue = std::move(rhs.m_acceptedValue);
    adoptChildren();
    return *this;
}

std::string RoseNode::debugDescription() const
{
    std::string desc;
    // TODO fix
    // if (!m_canonicalGenerator)
    //     desc += "<null>";
    // else
    //     desc += demangle(typeid(*m_canonicalGenerator).name());
    // desc += "[" + std::to_string(index()) + "]";
    return desc;
}

std::string RoseNode::debugPath() const
{
    if (m_parent == nullptr)
        return "/";
    else
        return m_parent->debugPath() + " " + debugDescription() + " /";
}

std::string RoseNode::debugIndexPath() const
{
    if (m_parent == nullptr)
        return "/";
    else
        return m_parent->debugIndexPath() + std::to_string(index()) + "/";
}

int RoseNode::depth() const
{
    if (m_parent == nullptr)
        return 0;
    else
        return m_parent->depth() + 1;
}

int RoseNode::index() const
{
    if (m_parent == nullptr) {
        return 0;
    } else {
        auto &parentChildren = m_parent->m_children;
        auto it = std::find_if(
            parentChildren.begin(), parentChildren.end(),
            [this] (const RoseNode &n) {
                return &n == this;
            });
        return it - parentChildren.begin();
    }
}

void RoseNode::adoptChildren()
{
    for (auto &child : m_children)
        child.m_parent = this;
}

} // namespace detail
} // namespace rc
