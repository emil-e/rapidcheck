#pragma once

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

} // namespace detail

template<typename T>
Seq<T> drop(std::size_t n, Seq<T> seq)
{ return makeSeq<detail::DropSeq<T>>(n, std::move(seq)); }

template<typename T>
Seq<T> take(std::size_t n, Seq<T> seq)
{ return makeSeq<detail::TakeSeq<T>>(n, std::move(seq)); }

template<typename Predicate, typename T>
Seq<T> dropWhile(Predicate &&pred, Seq<T> seq)
{
    return makeSeq<detail::DropWhileSeq<Decay<Predicate>, T>>(
        std::forward<Predicate>(pred), std::move(seq));
}

template<typename Predicate, typename T>
Seq<T> takeWhile(Predicate &&pred, Seq<T> seq)
{
    return makeSeq<detail::TakeWhileSeq<Decay<Predicate>, T>>(
        std::forward<Predicate>(pred), std::move(seq));
}

} // namespace seq
} // namespace rc
