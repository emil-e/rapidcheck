#pragma once

#include "rapidcheck/detail/Traits.h"

namespace rc {

template<typename T>
class Seq<T>::ISeqImpl
{
public:
    virtual bool hasNext() const = 0;
    virtual T next() = 0;
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

    bool hasNext() const override { return m_impl.hasNext(); }

    T next() override { return m_impl.next(); }

    std::unique_ptr<ISeqImpl> copy() const
    { return std::unique_ptr<ISeqImpl>(new SeqImpl(*this)); }

private:
    Impl m_impl;
};

template<typename T>
template<typename Impl, typename>
Seq<T>::Seq(Impl &&impl)
    : m_impl(impl.hasNext()
             ? new SeqImpl<detail::DecayT<Impl>>(std::forward<Impl>(impl))
             : nullptr) {}

template<typename T>
Seq<T>::operator bool() const
{ return m_impl && m_impl->hasNext(); }

template<typename T>
T Seq<T>::next()
{
    assert(m_impl);
    return m_impl->next();
}

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

} // namespace rc
