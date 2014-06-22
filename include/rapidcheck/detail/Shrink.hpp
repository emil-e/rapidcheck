#pragma once

#include "Utility.h"

namespace rc {
namespace shrink {

template<typename T>
class Nothing : public Iterator<T>
{
public:
    bool hasNext() const override { return false; }
    T next() { for (;;); } // Should never happen
};

template<typename T>
class RemoveChunks : public Iterator<T>
{
public:
    typedef typename T::size_type SizeT;

    RemoveChunks(T collection)
        : m_collection(std::move(collection))
        , m_skipStart(0)
        , m_skipSize(m_collection.size()) {}

    bool hasNext() const override
    { return m_skipSize != 0; }

    T next() override
    {
        T shrunk;
        SizeT i = 0;
        SizeT skipEnd = m_skipStart + m_skipSize;
        for (const auto &element : m_collection) {
            if ((i < m_skipStart) || (i >= skipEnd))
                shrunk.insert(shrunk.end(), element);
            i++;
        }

        m_skipStart++;
        if ((m_skipStart + m_skipSize) > m_collection.size()) {
            m_skipStart = 0;
            m_skipSize--;
        }

        return shrunk;
    }

private:
    T m_collection;
    SizeT m_skipStart;
    SizeT m_skipSize;
};

template<typename T, typename IteratorFactory>
class EachElement : public Iterator<T>
{
public:
    EachElement(T collection, IteratorFactory factory)
        : m_collection(std::move(collection))
        , m_factory(std::move(factory))
        , m_shrinkElement(m_collection.begin())
    {
        if (!m_collection.empty()) {
            m_shrinkIterator = m_factory(*m_shrinkElement);
            advance();
        }
    }

    bool hasNext() const override
    { return !m_collection.empty() && m_shrinkIterator->hasNext(); }

    T next() override
    {
        T shrunk;
        for (auto it = m_collection.begin(); it != m_collection.end(); it++)
        {
            if (it == m_shrinkElement)
                shrunk.insert(shrunk.end(), m_shrinkIterator->next());
            else
                shrunk.insert(shrunk.end(), *it);
        }

        advance();
        return shrunk;
    }

private:
    void advance()
    {
        while (!m_shrinkIterator->hasNext() &&
               (m_shrinkElement != m_collection.end()))
        {
            m_shrinkElement++;
            if (m_shrinkElement == m_collection.end())
                break;
            m_shrinkIterator = m_factory(*m_shrinkElement);
        }
    }

    T m_collection;
    IteratorFactory m_factory;
    IteratorUP<typename T::value_type> m_shrinkIterator;
    typename T::iterator m_shrinkElement;
};

template<typename T,
         typename I,
         typename Predicate,
         typename Iterate>
class Unfold : public Iterator<T>
{
public:
    Unfold(I initial, Predicate predicate, Iterate iterate)
        : m_it(std::move(initial))
        , m_predicate(std::move(predicate))
        , m_iterate(std::move(iterate)) {}

    bool hasNext() const override { return m_predicate(m_it); }

    T next() override
    {
        std::pair<T, I> result(m_iterate(m_it));
        m_it = result.second;
        return result.first;
    }

private:
    I m_it;
    Predicate m_predicate;
    Iterate m_iterate;
};

template<typename T>
class Sequentially : public Iterator<T>
{
public:
    typedef std::vector<IteratorUP<T>> Iterators;

    Sequentially(Iterators &&iterators)
        : m_iterators(std::move(iterators))
        , m_current(m_iterators.begin())
    {
        if (!m_iterators.empty())
            advance();
    }

    bool hasNext() const override
    { return m_current != m_iterators.end(); }

    T next() override
    {
        T value((*m_current)->next());
        advance();
        return value;
    }

private:
    void advance()
    {
        while ((m_current != m_iterators.end()) && !(*m_current)->hasNext())
            m_current++;
    }

    Iterators m_iterators;
    typename Iterators::iterator m_current;
};

template<typename T>
class Constant : public Iterator<T>
{
public:
    Constant(std::vector<T> constants)
        : m_constants(std::move(constants))
        , m_iterator(m_constants.begin()) {}

    bool hasNext() const override { return m_iterator != m_constants.end(); }
    T next() override { return *m_iterator++; }

private:
    std::vector<T> m_constants;
    typename std::vector<T>::iterator m_iterator;
};

template<typename T, typename Mapper>
class Mapped : public Iterator<
    typename std::result_of<Mapper(T)>::type>
{
public:
    Mapped(IteratorUP<T> &&iterator, Mapper mapper)
        : m_iterator(std::move(iterator))
        , m_mapper(std::move(mapper)) {}

    bool hasNext() const override { return m_iterator->hasNext(); }

    typename std::result_of<Mapper(T)>::type
    next() override { return m_mapper(m_iterator->next()); }

private:
    IteratorUP<T> m_iterator;
    Mapper m_mapper;
};

template<typename IteratorUP, typename ...IteratorsUP>
IteratorUP sequentially(IteratorUP &&iterator, IteratorsUP &&...iterators)
{
    std::vector<IteratorUP> iteratorVec;
    iteratorVec.reserve(1 + sizeof...(IteratorsUP));
    detail::pushBackAll(iteratorVec,
                        std::move(iterator),
                        std::move(iterators)...);
    typedef typename IteratorUP::element_type::ShrunkType T;
    return IteratorUP(new Sequentially<T>(std::move(iteratorVec)));
}

template<typename I,
         typename Predicate,
         typename Iterate>
IteratorUP<typename std::result_of<Iterate(I)>::type::first_type>
unfold(I initial, Predicate predicate, Iterate iterate)
{
    typedef typename decltype(iterate(initial))::first_type T;
    return IteratorUP<T>(new Unfold<T, I, Predicate, Iterate>(
                          std::move(initial),
                          std::move(predicate),
                          std::move(iterate)));
}

template<typename T>
IteratorUP<T> nothing()
{ return IteratorUP<T>(new Nothing<T>()); }

template<typename T, typename Mapper>
IteratorUP<typename std::result_of<Mapper(T)>::type>
map(IteratorUP<T> iterator, Mapper mapper)
{
    return IteratorUP<typename std::result_of<Mapper(T)>::type>(new Mapped<T, Mapper>(
                             std::move(iterator),
                             std::move(mapper)));
}

template<typename T>
IteratorUP<T> constant(std::vector<T> constants)
{ return IteratorUP<T>(new Constant<T>(std::move(constants))); }

template<typename T>
IteratorUP<T> removeChunks(T collection)
{ return IteratorUP<T>(new RemoveChunks<T>(std::move(collection))); }

template<typename T, typename IteratorFactory>
IteratorUP<T> eachElement(T collection, IteratorFactory factory)
{
    return IteratorUP<T>(new EachElement<T, IteratorFactory>(
                          std::move(collection),
                          std::move(factory)));
}

template<typename T>
IteratorUP<T> towards(T value, T target)
{
    return shrink::unfold(
        (value > target) ? (value - target) : (target - value),
        [=](T i) { return i != 0; },
        [=](T i) {
            T next = (value > target) ? (value - i) : (value + i);
            return std::make_pair(next, i / 2);
        });
}

} // namespace shrink
} // namespace rc
