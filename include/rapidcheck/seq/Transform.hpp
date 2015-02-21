#pragma once

#include "rapidcheck/detail/Meta.h"
#include "rapidcheck/seq/Create.h"

namespace rc {
namespace seq {
namespace detail {

template<typename T>
class DropSeq
{
public:
    DropSeq(std::size_t n, Seq<T> seq)
        : m_drop(n)
        , m_seq(std::move(seq)) {}

    Maybe<T> operator()()
    {
        while (m_drop > 0) {
            if (!m_seq.next()) {
                m_seq = Seq<T>();
                m_drop = 0;
                return Nothing;
            }
            m_drop--;
        }

        return m_seq.next();
    }

private:
    std::size_t m_drop;
    Seq<T> m_seq;
};

template<typename T>
class TakeSeq
{
public:
    TakeSeq(std::size_t n, Seq<T> seq)
        : m_take(n)
        , m_seq(std::move(seq)) {}

    Maybe<T> operator()()
    {
        if (m_take == 0)
            return Nothing;

        m_take--;
        return m_seq.next();
    }

private:
    std::size_t m_take;
    Seq<T> m_seq;
};

template<typename Predicate, typename T>
class DropWhileSeq
{
public:
    template<typename PredArg>
    DropWhileSeq(PredArg &&pred, Seq<T> seq)
        : m_pred(std::forward<PredArg>(pred))
        , m_dropped(false)
        , m_seq(std::move(seq)) {}

    Maybe<T> operator()()
    {
        while (!m_dropped) {
            auto value = m_seq.next();

            if (!value) {
                m_dropped = true;
                m_seq = Seq<T>();
                return Nothing;
            }

            if (!m_pred(*value)) {
                m_dropped = true;
                return value;
            }
        }

        return m_seq.next();
    }

private:
    Predicate m_pred;
    bool m_dropped;
    Seq<T> m_seq;
};

template<typename Predicate, typename T>
class TakeWhileSeq
{
public:
    template<typename PredArg>
    TakeWhileSeq(PredArg &&pred, Seq<T> seq)
        : m_pred(std::forward<PredArg>(pred))
        , m_seq(std::move(seq)) {}

    Maybe<T> operator()()
    {
        auto value = m_seq.next();
        if (!value)
            return Nothing;

        if (!m_pred(*value)) {
            m_seq = Seq<T>();
            return Nothing;
        }

        return value;
    }

private:
    Predicate m_pred;
    Seq<T> m_seq;
};

static inline bool allTrue() { return true; }

template<typename T, typename ...Ts>
bool allTrue(const T &arg, const Ts &...args)
{ return arg && allTrue(args...); }

template<typename Mapper, typename Indexes, typename ...Ts>
class MapSeq;

template<typename Mapper,
         std::size_t ...Indexes,
         typename ...Ts>
class MapSeq<Mapper, ::rc::detail::IndexSequence<Indexes...>, Ts...>
{
public:
    template<typename MapperArg>
    MapSeq(MapperArg &&mapper, Seq<Ts> ...seqs)
        : m_mapper(std::forward<MapperArg>(mapper))
        , m_seqs(std::move(seqs)...) {}

    Maybe<typename std::result_of<Mapper(Ts...)>::type> operator()()
    {
        std::tuple<Maybe<Ts>...> values(std::get<Indexes>(m_seqs).next()...);
        if (!allTrue(std::get<Indexes>(values)...)) {
            m_seqs = std::tuple<Seq<Ts>...>();
            return Nothing;
        }

        return m_mapper(std::move(*std::get<Indexes>(values))...);
    }

private:
    Mapper m_mapper;
    std::tuple<Seq<Ts>...> m_seqs;
};

template<typename Predicate, typename T>
class FilterSeq
{
public:
    template<typename PredArg>
    FilterSeq(PredArg predicate, Seq<T> seq)
        : m_predicate(std::forward<PredArg>(predicate))
        , m_seq(std::move(seq)) {}

    Maybe<T> operator()()
    {
        while (true) {
            auto value = m_seq.next();

            if (!value) {
                m_seq = Seq<T>();
                return Nothing;
            }

            if (m_predicate(*value))
                return value;
        }
    }

private:
    Predicate m_predicate;
    Seq<T> m_seq;
};

template<typename T>
class JoinSeq
{
public:
    JoinSeq(Seq<Seq<T>> seqs)
        : m_seqs(std::move(seqs)) {}

    Maybe<T> operator()()
    {
        while (true) {
            auto value = m_seq.next();
            if (value) {
                return value;
            }

            // Otherwise, next Seq
            auto seq = m_seqs.next();
            if (!seq) {
                m_seq = Seq<T>();
                m_seqs = Seq<Seq<T>>();
                return Nothing;
            }

            m_seq = std::move(*seq);
        }
    }

private:
    Seq<T> m_seq;
    Seq<Seq<T>> m_seqs;
};

} // namespace detail

template<typename T>
Seq<T> drop(std::size_t n, Seq<T> seq)
{
    if (n == 0)
        return seq;
    return makeSeq<detail::DropSeq<T>>(n, std::move(seq));
}

template<typename T>
Seq<T> take(std::size_t n, Seq<T> seq)
{
    if (n == 0)
        return Seq<T>();
    return makeSeq<detail::TakeSeq<T>>(n, std::move(seq));
}

template<typename T, typename Predicate>
Seq<T> dropWhile(Predicate &&pred, Seq<T> seq)
{
    return makeSeq<detail::DropWhileSeq<Decay<Predicate>, T>>(
        std::forward<Predicate>(pred), std::move(seq));
}

template<typename T, typename Predicate>
Seq<T> takeWhile(Predicate &&pred, Seq<T> seq)
{
    return makeSeq<detail::TakeWhileSeq<Decay<Predicate>, T>>(
        std::forward<Predicate>(pred), std::move(seq));
}

template<typename ...Ts, typename Mapper>
Seq<typename std::result_of<Mapper(Ts...)>::type>
map(Mapper &&mapper, Seq<Ts> ...seqs)
{
    typedef ::rc::detail::MakeIndexSequence<sizeof...(Ts)> Indexes;
    return makeSeq<detail::MapSeq<Decay<Mapper>, Indexes, Ts...>>(
        std::forward<Mapper>(mapper), std::move(seqs)...);
}

template<typename T, typename Predicate>
Seq<T> filter(Predicate &&pred, Seq<T> seq)
{
    return makeSeq<detail::FilterSeq<Decay<Predicate>, T>>(
        std::forward<Predicate>(pred), std::move(seq));
}

template<typename T>
Seq<T> join(Seq<Seq<T>> seqs)
{ return makeSeq<detail::JoinSeq<T>>(std::move(seqs)); }

template<typename T, typename ...Ts>
Seq<T> concat(Seq<T> seq, Seq<Ts> ...seqs)
{ return seq::join(seq::just(std::move(seq), std::move(seqs)...)); }

template<typename ...Ts, typename Mapper>
Seq<typename std::result_of<Mapper(Ts...)>::type::ValueType>
mapcat(Mapper &&mapper, Seq<Ts> ...seqs)
{ return seq::join(seq::map(std::forward<Mapper>(mapper), std::move(seqs)...)); }

template<typename T>
Seq<T> cycle(Seq<T> seq) { return seq::join(seq::repeat(std::move(seq))); }

template<typename T, typename U>
Seq<T> cast(Seq<U> seq)
{
    return seq::map(
        [](U &&x) { return static_cast<T>(std::move(x)); },
        std::move(seq));
}

} // namespace seq
} // namespace rc
