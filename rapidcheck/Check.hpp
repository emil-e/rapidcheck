#pragma once

#include "Generator.hpp"
#include "RandomEngine.hpp"
#include "ImplicitParam.hpp"
#include "Rose.hpp"

namespace rc {

template<typename T>
T pick(const Generator<T> &gen);

template<typename T>
T pick();

namespace detail {

//! Helper to extract arguments as a parameter pack.
//!
//! @param MemberFuncPtr  Type of the pointer to \c operator().
template<typename MemberFuncPtr> class FunctorHelper;

// Specialize to make argument deduction given us the parameter pack.
template<typename Functor, typename ...Args>
class FunctorHelper<bool (Functor::*)(Args...) const>
{
public:
    FunctorHelper(Functor functor) : m_functor(std::move(functor)) {}

    bool operator()() const
    { return m_functor(pick<typename std::decay<Args>::type>()...); }

private:
    Functor m_functor;
};

//! A \c Quantifier calls an underlying callable with parameters generated using
//! the \c pick<T> function.
//!
//! Please note that the base template is for functors, function pointers are
//! implemented through specialization of this template.
//!
//! @param Callable  The type of the callable.
template<typename Callable>
class Quantifier
{
public:
    Quantifier(Callable callable) : m_helper(std::move(callable)) {}
    bool operator()() const { return m_helper(); }

private:
    FunctorHelper<decltype(&Callable::operator())> m_helper;
};

// TODO support function pointers as well

} // namespace detail

template<typename T>
T pick(const Generator<T> &generator)
{
    return detail::RoseNode::pick(generator);
}

template<typename T>
T pick()
{
    return pick(arbitrary<T>());
}


template<typename Testable>
bool check(Testable testable)
{
    using namespace detail;
    ImplicitParam<param::Size> size;
    ImplicitParam<param::RandomEngine> randomEngine;

    RoseNode rootNode(0);
    size.let(50);
    randomEngine.let(RandomEngine());
    detail::Quantifier<Testable> property(testable);
    bool result = rootNode.call(property);
    rootNode.print(std::cout);
    return result;
}

}
