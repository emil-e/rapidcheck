#pragma once

namespace rc {
namespace detail {

//! Base template with no implementation, sepcialization is used to capture
//! arguments and return type of a member function pointer.
template<typename MemberFuncPtr> class FunctorHelper;

template<typename Functor, typename Ret, typename ...Args>
class FunctorHelper<Ret (Functor::*)(Args...) const>
{
public:
    typedef Ret ReturnType;

    explicit FunctorHelper(Functor functor) : m_functor(std::move(functor)) {}

    ReturnType operator()() const
    { return m_functor(pick<typename std::decay<Args>::type>()...); }

private:
    Functor m_functor;
};

//! Functor which calls `pick` for each argument of the given callable. Used to
//! implement quantification over function arguments.
//!
//! The base template is intented for functor and therefore inherits from
//! `FunctorHelper`. Function pointers are handled by a specialization.
template<typename Callable>
class Quantifier : public FunctorHelper<decltype(&Callable::operator())>
{
public:
    explicit Quantifier(Callable callable)
        : FunctorHelper<decltype(&Callable::operator())>(std::move(callable)) {}
};

//! Specialization for function pointers.
template<typename Ret, typename ...Args>
class Quantifier<Ret (*)(Args...)>
{
public:
    typedef Ret ReturnType;
    typedef ReturnType (*Function)(Args...);

    explicit Quantifier(Function function) : m_function(function) {}

    Ret operator()() const override
    { return m_function(pick<typename std::decay<Args>::type>()...); }

private:
    Function m_function;
};

} // namespace detail
} // namespace rc
