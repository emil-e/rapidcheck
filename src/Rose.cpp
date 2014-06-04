#include "rapidcheck/detail/Rose.h"

#include "rapidcheck/Generator.h"

namespace rc {
namespace detail {

UnexpectedTypeException::UnexpectedTypeException(
    const std::type_info &expected,
    const std::type_info &actual)
    : std::runtime_error(
        "Expected '" + demangle(expected.name())
        + "' but '" + demangle(actual.name()) + "' was requested")
    , m_expected(expected)
    , m_actual(actual) {}

RoseNode::RoseNode() : RoseNode(nullptr) {}

RandomEngine::Atom RoseNode::atom()
{
    if (!m_hasAtom) {
        ImplicitParam<param::RandomEngine> randomEngine;
        m_atom = randomEngine->nextAtom();
        m_hasAtom = true;
    }

    return m_atom;
}

void RoseNode::print(std::ostream &os)
{
    for (int i = 0; i < depth(); i++)
        os << "  ";
    os << "- " << description() << std::endl;

    for (auto &child : m_children)
        child.print(os);
}

std::vector<gen::ValueDescription> RoseNode::example()
{
    // TODO we should capture values during generation instead, somehow...
    std::vector<gen::ValueDescription> values;
    values.reserve(m_children.size());
    for (auto &child : m_children)
        values.push_back(child.regenerateDescription());
    return values;
}

gen::ValueDescription RoseNode::regenerateDescription()
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

bool RoseNode::isFrozen() const
{
    return bool(m_acceptedGenerator);
}

RoseNode::RoseNode(RoseNode &&other)
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

RoseNode &RoseNode::operator=(RoseNode &&rhs)
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

RoseNode &RoseNode::current()
{ return **ImplicitParam<CurrentNode>(); }

    //! Returns a reference to the current node.
bool RoseNode::hasCurrent()
{ return ImplicitParam<CurrentNode>().hasBinding(); }

RoseNode::RoseNode(RoseNode *parent) : m_parent(parent) {}

int RoseNode::depth() const
{
    if (m_parent == nullptr)
        return 0;

    return m_parent->depth() + 1;
}

void RoseNode::adoptChildren()
{
    for (auto &child : m_children)
        child.m_parent = this;
}

std::string RoseNode::description() const
{
    std::string desc(generatorName());
    if (m_parent != nullptr)
        desc += "[" + std::to_string(index()) + "]";
    return desc;
}

std::ptrdiff_t RoseNode::index() const
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

std::string RoseNode::path()
{
    if (m_parent == nullptr)
        return "/ " + description() + "\n";
    else
        return m_parent->path() + "/ " + description() + "\n";
}

gen::UntypedGenerator *RoseNode::activeGenerator() const
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

std::string RoseNode::generatorName() const
{
    auto gen = activeGenerator();
    if (gen == nullptr)
        return std::string();
    else
        return demangle(typeid(*gen).name());
}

void RoseNode::acceptShrink()
{
    if (!m_currentGenerator)
        return;
    m_acceptedGenerator = std::move(m_currentGenerator);
    m_shrinkIterator = nullptr;
}

} // namespace detail
} // namespace rc
