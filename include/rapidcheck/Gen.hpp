#pragma once

#include <cassert>

#include "rapidcheck/detail/Any.h"
#include "rapidcheck/detail/ImplicitParam.h"
#include "rapidcheck/gen/detail/GenerationHandler.h"
#include "rapidcheck/shrinkable/Create.h"

namespace rc {
namespace gen {

// Forward declare this so we don't need to include Transform.h
template <typename T, typename Mapper>
Gen<Decay<typename std::result_of<Mapper(T)>::type>> map(
    Gen<T> gen, Mapper &&mapper);

} // namespace gen

template <typename T>
class Gen<T>::IGenImpl {
public:
  virtual Shrinkable<T> generate(const Random &random, int size) const = 0;
  virtual std::unique_ptr<IGenImpl> copy() const = 0;
  virtual ~IGenImpl() = default;
};

template <typename T>
template <typename Impl>
class Gen<T>::GenImpl : public IGenImpl {
public:
  template <typename... Args>
  GenImpl(Args &&... args)
      : m_impl(std::forward<Args>(args)...) {}

  Shrinkable<T> generate(const Random &random, int size) const override {
    return m_impl(random, size);
  }

  std::unique_ptr<IGenImpl> copy() const override {
    return std::unique_ptr<IGenImpl>(new GenImpl(*this));
  }

private:
  const Impl m_impl;
};

template <typename T>
template <typename Impl, typename>
Gen<T>::Gen(Impl &&impl)
    : m_impl(new GenImpl<Decay<Impl>>(std::forward<Impl>(impl))) {}

template <typename T>
Shrinkable<T> Gen<T>::operator()(const Random &random, int size) const
    noexcept {
  try {
    return m_impl->generate(random, size);
  } catch (...) {
    auto exception = std::current_exception();
    return shrinkable::lambda(
        [=]() -> T { std::rethrow_exception(exception); });
  }
}

template <typename T>
T Gen<T>::operator*() const {
  using namespace detail;
  using rc::gen::detail::param::CurrentHandler;
  const auto handler = ImplicitParam<CurrentHandler>::value();
  return std::move(
      handler->onGenerate(gen::map(*this, &Any::of<T>)).template get<T>());
}

template <typename T>
Gen<T>::Gen(const Gen &other)
    : m_impl(other.m_impl->copy()) {}

template <typename T>
Gen<T> &Gen<T>::operator=(const Gen &rhs) {
  m_impl = rhs.m_impl->copy();
  return *this;
}

} // namespace rc
