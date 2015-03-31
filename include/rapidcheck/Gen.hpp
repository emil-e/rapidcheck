#pragma once

#include <cassert>

#include "rapidcheck/detail/Any.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/newgen/detail/GenerationHandler.h"

namespace rc {
namespace newgen {

// Forward declare this so we don't need to include Transform.h
template<typename T, typename Mapper>
Gen<Decay<typename std::result_of<Mapper(T)>::type>>
map(Gen<T> gen, Mapper &&mapper);

} // namespace newgen

template<typename T>
class Gen<T>::IGenImpl
{
public:
    virtual Shrinkable<T> generate(const Random &random, int size) const = 0;
    virtual std::unique_ptr<IGenImpl> copy() const = 0;
    virtual ~IGenImpl() = default;
};

template<typename T>
template<typename Impl>
class Gen<T>::GenImpl : public IGenImpl
{
public:
    template<typename ...Args>
    GenImpl(Args &&...args) : m_impl(std::forward<Args>(args)...) {}

    Shrinkable<T> generate(const Random &random, int size) const override
    { return m_impl(random, size); }

    std::unique_ptr<IGenImpl> copy() const override
    { return std::unique_ptr<IGenImpl>(new GenImpl(*this)); }

private:
    const Impl m_impl;
};

template<typename T>
template<typename Impl, typename>
Gen<T>::Gen(Impl &&impl)
    : m_impl(new GenImpl<Decay<Impl>>(std::forward<Impl>(impl))) {}

template<typename T>
Shrinkable<T> Gen<T>::operator()(const Random &random, int size) const
{ return m_impl->generate(random, size); }

template<typename T>
T Gen<T>::operator*() const
{
    using namespace detail;
    using rc::newgen::detail::param::CurrentHandler;
    const auto handler = ImplicitParam<CurrentHandler>::value();
    return std::move(
        handler->onGenerate(newgen::map(*this, &Any::of<T>)).template get<T>());
}

template<typename T>
Gen<T>::Gen(const Gen &other) : m_impl(other.m_impl->copy()) {}

template<typename T>
Gen<T> &Gen<T>::operator=(const Gen &rhs)
{
    m_impl = rhs.m_impl->copy();
    return *this;
}

} // namespace rc
