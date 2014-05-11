#pragma once

namespace rc {

template<typename T> struct Shrinkable;

//! The shrinker for values of type \c T.
template<typename T>
using Shrinker = typename std::result_of<
    decltype(&Shrinkable<T>::shrink)(T)>::type;

//! Combines multiple shrinkers. The shrinkers all need to have a single argument
//! constructor taking the value to be shrunk.
template<typename T, typename ...Shrinkers> class ShrinkerList;

//! Base case for \c ShrinkerList. Also works nicely as a "null" shrinker.
template<typename T>
class ShrinkerList<T>
{
public:
    ShrinkerList(const T &) {};

    // Always false
    bool hasNext() const { return false; }
    // Never returns but should never be called
    T next() { for(;;); };
};

template<typename T, typename X, typename ...Xs>
class ShrinkerList<T, X, Xs...>
{
public:
    ShrinkerList(T value) : m_head(value), m_tail(value)
    {
        static_assert(std::is_constructible<X, T>::value,
                  "Shrinker must have single argument constructor taking the"
                  "value to be shrunk");
    }

    bool hasNext() const { return m_head.hasNext() || m_tail.hasNext(); }

    T next() { return m_head.hasNext() ? m_head.next() : m_tail.next(); }

private:
    X m_head;
    ShrinkerList<T, Xs...> m_tail;
};

//! Returns constant values in succession.
// template<typename T>
// class ConstantShrinker
// {
// public:
//     ConstantShrinker(std::initializer_list<T> values)
//         : m_values(values)
//         , m_it(m_values.begin()) {}

//     bool hasNext() const { return m_it != m_values.end(); }
//     T next() { return *it++; }

// private:
//     std::vector<T> m_values;
//     std::vector<T>::iterator m_it;
// }

//! Shrinker for integer types.
template<typename T>
class IntegerDividerShrinker
{
public:
    IntegerDividerShrinker(T value) : m_currentValue(value) {}

    bool hasNext() const { return m_currentValue != 0; }

    T next()
    {
        m_currentValue /= 2;
        return m_currentValue;
    }

private:
    T m_currentValue;
};

//! Shrinks collections by removal of each element in turn.
template<typename T>
class RemoveElementShrinker
{
public:
    RemoveElementShrinker(T collection)
        : m_collection(std::move(collection))
    { m_skipElement = m_collection.begin(); }

    bool hasNext() const { return m_skipElement != m_collection.end(); }

    T next()
    {
        T shrunk;
        for (auto it = m_collection.begin(); it != m_collection.end(); it++) {
            if (it != m_skipElement)
                shrunk.insert(shrunk.end(), *it);
        }

        m_skipElement++;
        return shrunk;
    }

private:
    typename T::const_iterator m_skipElement;
    T m_collection;
};

//! Shrinks collections by trying to shrink each element in turn.
template<typename T>
class ShrinkElementShrinker
{
public:
    typedef typename T::value_type Element;
    typedef Shrinker<Element> ElementShrinker;

    ShrinkElementShrinker(T collection)
        : m_collection(std::move(collection))
        , m_shrinkElement(m_collection.begin())
    {
        if (!m_collection.empty()) {
            m_elementShrinker = std::unique_ptr<ElementShrinker>(
                new ElementShrinker(
                    Shrinkable<Element>::shrink(*m_shrinkElement)));
            advance();
        }
    }

    bool hasNext() const
    { return !m_collection.empty() && m_elementShrinker->hasNext(); }

    T next()
    {
        T shrunk;
        for (auto it = m_collection.begin(); it != m_collection.end(); it++)
        {
            if (it == m_shrinkElement)
                shrunk.insert(shrunk.end(), m_elementShrinker->next());
            else
                shrunk.insert(shrunk.end(), *it);
        }

        advance();
        return shrunk;
    }

private:
    void advance()
    {
        while (!m_elementShrinker->hasNext() &&
               (m_shrinkElement != m_collection.end()))
        {
            m_shrinkElement++;
            if (m_shrinkElement == m_collection.end())
                break;
            *m_elementShrinker = Shrinkable<Element>::shrink(*m_shrinkElement);
        }
    }

    T m_collection;
    typename T::const_iterator m_shrinkElement;
    // We must use a pointer here since ElementShrinker is probably not default
    // constructible.
    std::unique_ptr<ElementShrinker> m_elementShrinker;
};

//! A collection shrinker is a combination of first trying to shrink by removing
//! elements and then trying to shrink each elements.
template<typename T>
using CollectionShrinker = ShrinkerList<T, RemoveElementShrinker<T>,
                                        ShrinkElementShrinker<T>>;

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value, ShrinkerList<T>>::type
defaultShrink(T value)
{
    return ShrinkerList<T>(value);
}

//! For integers.
template<typename T>
typename std::enable_if<std::is_integral<T>::value, IntegerDividerShrinker<T>>::type
defaultShrink(T value)
{
    return IntegerDividerShrinker<T>(value);
}

//! Specialize this class to provide shrinking for your own types. The
//! specialization should have a method \c shrink which returns an object that
//! provides possible shrinks successively through a \c next method. It should
//! also provide a \c hasNext method that returns \c true if there is a next
//! shrink and \c false if there is not.
template<typename T>
struct Shrinkable
{
    typedef decltype(defaultShrink(std::declval<T>())) ShrinkerT;

    static ShrinkerT shrink(const T &value)
    { return defaultShrink(value); }
};

template<typename T>
struct Shrinkable<std::vector<T>>
{
    static CollectionShrinker<std::vector<T>> shrink(std::vector<T> vec)
    { return CollectionShrinker<std::vector<T>>(std::move(vec)); }
};

} // namespace rc
