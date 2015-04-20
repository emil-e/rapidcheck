#pragma once

namespace rc {

template<typename T>
class Shrinkable<T>::IShrinkableImpl
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

private:
    Impl m_impl;
};

template<typename T>
T Shrinkable<T>::value() const { return m_impl->value(); }

template<typename T>
Seq<Shrinkable<T>> Shrinkable<T>::shrinks() const noexcept
{
    try {
        return m_impl->shrinks();
    } catch (...) {
        return Seq<Shrinkable<T>>();
    }
}

template<typename T>
Shrinkable<T>::Shrinkable(std::shared_ptr<IShrinkableImpl> impl)
    : m_impl(std::move(impl)) {}

template<typename Impl, typename ...Args>
Shrinkable<Decay<decltype(std::declval<Impl>().value())>>
makeShrinkable(Args &&...args)
{
    typedef decltype(std::declval<Impl>().value()) T;
    typedef typename Shrinkable<T>::IShrinkableImpl IShrinkableImpl;
    typedef typename Shrinkable<T>::template ShrinkableImpl<Impl> ShrinkableImpl;

    return Shrinkable<T>(
        std::shared_ptr<IShrinkableImpl>(
            std::make_shared<ShrinkableImpl>(std::forward<Args>(args)...)));
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
