#pragma once

#include "rapidcheck/Show.h"

namespace rc {

template<typename T>
class Seq<T>::ISeqImpl : public detail::IPolymorphic
{
public:
    virtual Maybe<T> next() = 0;
    virtual ~ISeqImpl() = default;
};

template<typename T>
class Seq<T>::EmptySeqImpl : public Seq<T>::ISeqImpl
{
public:
    Maybe<T> next() override { return Nothing; }

    void copyInto(void *storage) const override
    { new (storage) EmptySeqImpl(); }

    void moveInto(void *storage) override
    { new (storage) EmptySeqImpl(); }
};

template<typename T>
template<typename Impl>
class Seq<T>::HeapSeqImpl : public Seq<T>::ISeqImpl
{
public:
    template<typename ...Args>
    HeapSeqImpl(Args &&...args)
        : m_impl(new Impl(std::forward<Args>(args)...)) {}

    Maybe<T> next() override
    { return m_impl ? (*m_impl)() : Maybe<T>(); }

    void copyInto(void *storage) const override
    {
        if (m_impl)
            new (storage) HeapSeqImpl(*m_impl);
        else
            new (storage) EmptySeqImpl();
    }

    void moveInto(void *storage) override
    {
        if (m_impl)
            new (storage) HeapSeqImpl(std::move(*this));
        else
            new (storage) EmptySeqImpl();
    }

private:
    std::unique_ptr<Impl> m_impl;
};

template<typename T>
template<typename Impl>
class Seq<T>::LocalSeqImpl : public Seq<T>::ISeqImpl
{
public:
    template<typename ...Args>
    LocalSeqImpl(Args &&...args)
        : m_impl(std::forward<Args>(args)...) {}

    Maybe<T> next() override { return m_impl(); }

    void copyInto(void *storage) const override
    { new (storage) LocalSeqImpl(*this); }

    void moveInto(void *storage) override
    { new (storage) LocalSeqImpl(std::move(*this)); }

private:
    Impl m_impl;
};

template<typename T>
Seq<T>::Seq() noexcept { m_storage.template init<EmptySeqImpl>(); }

template<typename T>
template<typename Impl, typename>
Seq<T>::Seq(Impl &&impl)
{
    using LocalImpl = LocalSeqImpl<Decay<Impl>>;
    using HeapImpl = HeapSeqImpl<Decay<Impl>>;

    m_storage.template initWithFallback<LocalImpl, HeapImpl>(
        std::forward<Impl>(impl));
}

template<typename T>
Seq<T>::Seq(std::false_type) {}

template<typename T>
Maybe<T> Seq<T>::next() noexcept
{
    try {
        return m_storage.get<ISeqImpl>().next();
    } catch (...) {
        return Nothing;
    }
}

template<typename Impl, typename ...Args>
Seq<typename std::result_of<Impl()>::type::ValueType> makeSeq(Args &&...args)
{
    typedef Seq<typename std::result_of<Impl()>::type::ValueType> SeqT;
    using LocalImpl = typename SeqT::template LocalSeqImpl<Impl>;
    using HeapImpl = typename SeqT::template HeapSeqImpl<Impl>;

    SeqT seq{std::false_type()};
    seq.m_storage.template initWithFallback<LocalImpl, HeapImpl>(
        std::forward<Args>(args)...);
    return seq;
}

template<typename T>
bool operator==(Seq<T> lhs, Seq<T> rhs)
{
    while (true) {
        Maybe<T> a(lhs.next());
        Maybe<T> b(rhs.next());
        if (a != b)
            return false;

        if (!a && !b)
            return true;
    }
}

template<typename T>
bool operator!=(Seq<T> lhs, Seq<T> rhs)
{ return !(std::move(lhs) == std::move(rhs)); }

template<typename T>
std::ostream &operator<<(std::ostream &os, Seq<T> seq)
{
    os << "[";
    int n = 1;
    auto first = seq.next();
    if (first) {
        show(*first, os);
        while (true) {
            const auto value = seq.next();
            if (!value)
                break;

            os << ", ";
            // Don't print infinite sequences...
            if (n++ >= 1000) {
                os << "...";
                break;
            }

            show(*value, os);
        }
    }
    os << "]";
    return os;
}

} // namespace rc
