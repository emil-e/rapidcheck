#pragma once

namespace rc {

template<typename T>
class NullIterator : public ShrinkIterator<T>
{
public:
    bool hasNext() const override { return false; }
    T next() { for (;;); } // Should never happen
};

template<typename T>
class RemoveChunksIterator : public ShrinkIterator<T>
{
public:
    typedef typename T::size_type SizeT;

    RemoveChunksIterator(T collection)
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

template<typename T,
         typename I,
         typename Predicate,
         typename Iterate>
class UnfoldIterator : public ShrinkIterator<T>
{
public:
    UnfoldIterator(I initial, Predicate predicate, Iterate iterate)
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

template<typename I,
         typename Predicate,
         typename Iterate>
ShrinkIteratorUP<typename std::result_of<Iterate(I)>::type::first_type>
unfold(I initial, Predicate predicate, Iterate iterate)
{
    typedef typename decltype(iterate(initial))::first_type T;
    return ShrinkIteratorUP<T>(new UnfoldIterator<T, I, Predicate, Iterate>(
                                   std::move(initial),
                                   std::move(predicate),
                                   std::move(iterate)));
}

template<typename T>
ShrinkIteratorUP<T> shrinkNothing()
{
    return ShrinkIteratorUP<T>(new NullIterator<T>());
}

} // namespace rc
