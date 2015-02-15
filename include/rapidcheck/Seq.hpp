#pragma once

namespace rc {

template<typename T>
class Seq<T>::ISeqImpl
{
public:
    virtual Maybe<T> next() = 0;
    virtual std::unique_ptr<ISeqImpl> copy() const = 0;
    virtual ~ISeqImpl() = default;
};

template<typename T>
template<typename Impl>
class Seq<T>::SeqImpl : public Seq<T>::ISeqImpl
{
public:
    template<typename Arg>
    SeqImpl(Arg &&impl) : m_impl(std::forward<Arg>(impl)) {}

    Maybe<T> next() override { return m_impl(); }

    std::unique_ptr<ISeqImpl> copy() const
    { return std::unique_ptr<ISeqImpl>(new SeqImpl(*this)); }

private:
    Impl m_impl;
};

template<typename T>
template<typename Impl, typename>
Seq<T>::Seq(Impl &&impl)
    : m_impl(new SeqImpl<Decay<Impl>>(std::forward<Impl>(impl))) {}

template<typename T>
Maybe<T> Seq<T>::next()
{ return m_impl ? m_impl->next() : Nothing; }

template<typename T>
Seq<T>::Seq(const Seq &other)
    : m_impl(other.m_impl ? other.m_impl->copy() : nullptr) {}

template<typename T>
Seq<T> &Seq<T>::operator=(const Seq &rhs)
{
    m_impl = rhs.m_impl->copy();
    return *this;
}

template<typename T>
Seq<T>::Seq(Seq &&other)
    : m_impl(std::move(other.m_impl)) {}

template<typename T>
Seq<T> &Seq<T>::operator=(Seq &&rhs)
{
    m_impl = std::move(rhs.m_impl);
    return *this;
}

template<typename A, typename B>
bool operator==(Seq<A> lhs, Seq<B> rhs)
{
    while (true) {
        Maybe<A> a(lhs.next());
        Maybe<B> b(rhs.next());
        if (a != b)
            return false;

        if (!a && !b)
            return true;
    }
}

template<typename A, typename B>
bool operator!=(Seq<A> lhs, Seq<B> rhs)
{ return !(std::move(lhs) == std::move(rhs)); }

} // namespace rc
