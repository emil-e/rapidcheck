#pragma once

namespace rc {

template<typename T>
class Shrinkable<T>::IShrinkableImpl
{
public:
    virtual T value() const = 0;
    virtual Seq<Shrinkable<T>> shrinks() const = 0;
    virtual std::unique_ptr<IShrinkableImpl> copy() const = 0;
    virtual ~IShrinkableImpl() = default;
};

template<typename T>
template<typename Impl>
class Shrinkable<T>::ShrinkableImpl : public IShrinkableImpl
{
public:
    template<typename ...Args>
    explicit ShrinkableImpl(Args &&...args)
        : m_impl(std::forward<Args>(args)...) {}

    T value() const override { return m_impl.value(); }
    Seq<Shrinkable<T>> shrinks() const override { return m_impl.shrinks(); }

    std::unique_ptr<IShrinkableImpl> copy() const override
    { return std::unique_ptr<IShrinkableImpl>(new ShrinkableImpl(m_impl)); }

private:
    Impl m_impl;
};

template<typename T>
template<typename Impl, typename>
Shrinkable<T>::Shrinkable(Impl &&impl)
    : m_impl(new ShrinkableImpl<Decay<Impl>>(std::forward<Impl>(impl))) {}

template<typename T>
T Shrinkable<T>::value() const { return m_impl->value(); }

template<typename T>
Seq<Shrinkable<T>> Shrinkable<T>::shrinks() const { return m_impl->shrinks(); }

template<typename T>
Shrinkable<T>::Shrinkable(const Shrinkable &other)
    : m_impl(other.m_impl->copy()) {}

template<typename T>
Shrinkable<T> &Shrinkable<T>::operator=(const Shrinkable &rhs)
{
    m_impl = rhs.m_impl->copy();
    return *this;
}

template<typename T>
Shrinkable<T>::Shrinkable(std::unique_ptr<IShrinkableImpl> impl)
    : m_impl(std::move(impl)) {}

template<typename Impl, typename ...Args>
Shrinkable<Decay<decltype(std::declval<Impl>().value())>>
makeShrinkable(Args &&...args)
{
    typedef decltype(std::declval<Impl>().value()) T;
    typedef typename Shrinkable<T>::IShrinkableImpl IShrinkableImpl;
    typedef typename Shrinkable<T>::template ShrinkableImpl<Impl> ShrinkableImpl;

    return Shrinkable<T>(
        std::unique_ptr<IShrinkableImpl>(
            new ShrinkableImpl(std::forward<Args>(args)...)));
}

template<typename T>
bool operator==(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs)
{
    return
        (lhs.value() == rhs.value()) &&
        (lhs.shrinks() == rhs.shrinks());
}

template<typename T>
bool operator!=(const Shrinkable<T> &lhs, const Shrinkable<T> &rhs)
{ return !(lhs == rhs); }

} // namespace rc
