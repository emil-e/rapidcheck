#pragma once

#include <iostream>

#include "rapidcheck/detail/Traits.h"
#include "rapidcheck/detail/CollectionBuilder.h"
#include "rapidcheck/detail/Utility.h"

namespace rc {
namespace shrink {

template<typename T>
class Nothing : public Iterator<T>
{
public:
    bool hasNext() const override { return false; }
    T next() { for (;;); } // Should never happen
};

template<typename Container>
class RemoveChunks : public Iterator<Container>
{
public:
    static_assert(detail::IsCopyConstructible<Container>::value,
                  "Element type must be copy constructible");

    typedef typename Container::size_type SizeT;

    RemoveChunks(Container collection)
        : m_collection(std::move(collection))
        , m_size(std::distance(begin(m_collection), end(m_collection)))
        , m_skipStart(0)
        , m_skipSize(m_size)
    {}

    bool hasNext() const override
    { return m_skipSize != 0; }

    Container next() override
    {
        detail::CollectionBuilder<Container> builder;
        SizeT i = 0;
        SizeT skipEnd = m_skipStart + m_skipSize;
        for (const auto &element : m_collection) {
            if ((i < m_skipStart) || (i >= skipEnd))
                builder.add(element);
            i++;
        }

        m_skipStart++;
        if (skipEnd >= m_size) {
            m_skipStart = 0;
            m_skipSize--;
        }

        return std::move(builder.result());
    }

private:
    Container m_collection;
    SizeT m_size;
    SizeT m_skipStart;
    SizeT m_skipSize;
};

template<typename Container, typename IteratorFactory>
class EachElement : public Iterator<Container>
{
public:
    static_assert(detail::IsCopyConstructible<Container>::value,
                  "Element type must be copy constructible");

    typedef typename std::result_of<
        IteratorFactory(typename Container::value_type)>::type::element_type::ShrunkType
        ElementT;

    EachElement(Container collection, IteratorFactory factory)
        : m_collection(std::move(collection))
        , m_factory(std::move(factory))
        , m_shrinkElement(m_collection.begin())
    { advance(); }

    bool hasNext() const override
    { return m_shrinkElement != m_collection.end(); }

    Container next() override
    {
        Container c(std::move(m_next));
        advance();
        return std::move(c);
    }

private:
    void advance()
    {
        while (m_shrinkElement != m_collection.end()) {
            if (tryAdvance())
                return;
        }
    }

    bool tryAdvance()
    {
        detail::CollectionBuilder<Container> builder;
        for (auto it = m_collection.begin(); it != m_collection.end(); it++)
        {
            if (it == m_shrinkElement) {
                if (!m_elementIterator)
                    m_elementIterator = m_factory(*it);

                if (m_elementIterator->hasNext()) {
                    if (!builder.add(m_elementIterator->next()))
                        return false;
                    else
                        continue;
                } else {
                    m_elementIterator = nullptr;
                    m_shrinkElement++;
                }
            }

            if (!builder.add(*it))
                return false;
        }

        m_next = std::move(builder.result());
        return true;
    }

    Container m_collection;
    Container m_next;
    IteratorFactory m_factory;
    IteratorUP<ElementT> m_elementIterator;
    typename Container::iterator m_shrinkElement;
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
        std::pair<T, I> result(m_iterate(std::move(m_it)));
        m_it = std::move(result.second);
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

template<typename T, typename Predicate>
class Filter : public Iterator<T>
{
public:
    Filter(IteratorUP<T> &&iterator, Predicate predicate)
        : m_iterator(std::move(iterator))
        , m_predicate(std::move(predicate))
    { advance(); }

    bool hasNext() const override { return bool(m_next); }

    T next() override
    {
        T next(std::move(*m_next));
        advance();
        return std::move(next);
    }

private:
    void advance()
    {
        while (m_iterator->hasNext()) {
            if (m_next)
                *m_next = m_iterator->next();
            else
                m_next = std::unique_ptr<T>(new T(m_iterator->next()));

            if (m_predicate(*m_next))
                return;
        }

        m_next = nullptr;
    }

    IteratorUP<T> m_iterator;
    std::unique_ptr<T> m_next;
    Predicate m_predicate;
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
    typedef typename std::make_unsigned<T>::type Uint;
    return shrink::unfold(
        static_cast<Uint>((value > target)
                          ? (value - target)
                          : (target - value)),
        [=](Uint i) { return i != 0; },
        [=](Uint i) {
            T next = (value > target) ? (value - i) : (value + i);
            return std::make_pair(next, i / 2);
        });
}

template<typename T, typename Predicate>
IteratorUP<T> filter(IteratorUP<T> &&iterator, Predicate predicate)
{
    return IteratorUP<T>(new Filter<T, Predicate>(
                             std::move(iterator),
                             std::move(predicate)));
}

} // namespace shrink
} // namespace rc
