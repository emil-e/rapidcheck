#pragma once

#include "Any.h"
#include "rapidcheck/detail/GeneratorFwd.h"

namespace rc {
namespace detail {

//! Wraps a reference to a generator and erases its type to be `Any` which is
//! useful to reduce template instantiation in internal code that deals with
//! generators.
template<typename T>
class ErasedGenerator : public gen::Generator<Any> {
public:
    ErasedGenerator(const gen::Generator<T> &generator);
    Any generate() const;
    shrink::IteratorUP<Any> shrink(Any value) const;

private:
    const gen::Generator<T> &m_generator;
};

} // namespace detail
} // namespace rc

#include "ErasedGenerator.hpp"
