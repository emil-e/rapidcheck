#pragma once

namespace rc {
namespace detail {

// BaseBuilder

template<typename Container>
Container &BaseBuilder<Container>::result()
{ return m_container; }

// PushBackBuilder

template<typename Container>
template<typename T>
bool EmplaceBackBuilder<Container>::add(T &&value)
{
    this->m_container.emplace_back(std::forward<T>(value));
    return true;
}

// InsertEndBuilder

template<typename Container>
template<typename T>
bool InsertEndBuilder<Container>::add(T &&value)
{
    this->m_container.insert(this->m_container.end(), std::forward<T>(value));
    return true;
}

// InsertAfterBuilder

template<typename Container>
InsertAfterBuilder<Container>::InsertAfterBuilder()
    : m_iterator(this->m_container.before_begin()) {}

template<typename Container>
template<typename T>
bool InsertAfterBuilder<Container>::add(T &&value)
{
    m_iterator = this->m_container.insert_after(m_iterator,
                                                std::forward<T>(value));
    return true;
}

// ArrayBuilder

template<typename Container>
ArrayBuilder<Container>::ArrayBuilder()
    : m_iterator(this->m_container.begin()) {}

template<typename Container>
template<typename T>
bool ArrayBuilder<Container>::add(T &&value)
{
    if (m_iterator == this->m_container.end())
        return false;

    *m_iterator = std::forward<T>(value);
    m_iterator++;
    return true;
}

// InsertKeyMaybeBuilder

template<typename Container>
template<typename T>
bool InsertKeyMaybeBuilder<Container>::add(T &&value)
{ return this->m_container.insert(std::forward<T>(value)); }

template<typename Container>
template<typename T>
bool InsertPairMaybeBuilder<Container>::add(T &&value)
{
    auto result = this->m_container.emplace(std::forward<T>(value));
    return result.second;
}

} // namespace detail
} // namespace rc
