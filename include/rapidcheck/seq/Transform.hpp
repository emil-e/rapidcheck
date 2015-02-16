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

} // namespace detail

template<typename T>
Seq<T> drop(std::size_t n, Seq<T> seq)
{ return makeSeq<detail::DropSeq<T>>(n, std::move(seq)); }

} // namespace seq
} // namespace rc
