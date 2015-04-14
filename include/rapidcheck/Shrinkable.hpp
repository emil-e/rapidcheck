#pragma once

#include "rapidcheck/detail/ShowType.h"

namespace rc {

template<typename T>
class Shrinkable<T>::IShrinkableImpl : public detail::IPolymorphic
{
public:
    virtual T value() const = 0;
    virtual Seq<Shrinkable<T>> shrinks() const = 0;
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

    void copyInto(void *storage) const override
    { new (storage) ShrinkableImpl(*this); }

    void moveInto(void *storage) override
    { new (storage) ShrinkableImpl(std::move(*this)); }

private:
    Impl m_impl;
};

template<typename T>
template<typename Impl>
class Shrinkable<T>::SharedShrinkableImpl : public IShrinkableImpl
{
public:
    template<typename ...Args>
    explicit SharedShrinkableImpl(Args &&...args)
        : m_impl(std::make_shared<Impl>(std::forward<Args>(args)...)) {}

    T value() const override { return m_impl->value(); }
    Seq<Shrinkable<T>> shrinks() const override { return m_impl->shrinks(); }

    void copyInto(void *storage) const override
    { new (storage) SharedShrinkableImpl(*this); }

    void moveInto(void *storage) override
    { new (storage) SharedShrinkableImpl(std::move(*this)); }

private:
    std::shared_ptr<const Impl> m_impl;
};

template<typename T>
T Shrinkable<T>::value() const
{ return m_storage.get<IShrinkableImpl>().value(); }

template<typename T>
Seq<Shrinkable<T>> Shrinkable<T>::shrinks() const noexcept
{
    try {
        return m_storage.get<IShrinkableImpl>().shrinks();
    } catch (...) {
        return Seq<Shrinkable<T>>();
    }
}

template<typename Impl, typename ...Args>
Shrinkable<Decay<decltype(std::declval<Impl>().value())>>
makeShrinkable(Args &&...args)
{
    using T = decltype(std::declval<Impl>().value());
    using NonShared = typename Shrinkable<T>::template ShrinkableImpl<Impl>;
    using Shared = typename Shrinkable<T>::template SharedShrinkableImpl<Impl>;

    Shrinkable<T> shrinkable;
    shrinkable.m_storage.template initWithFallback<NonShared, Shared>(
        std::forward<Args>(args)...);
    return shrinkable;
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
