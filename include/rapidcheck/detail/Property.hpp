#pragma once

namespace rc {
namespace detail {

static inline Result toResult(bool value)
{
    return value ? Result::Success : Result::Failure;
}

//! Helper class to convert different return types to `Result`.
template<typename Callable,
         typename ReturnType = typename std::result_of<Callable()>::type>
struct ResultHelper
{
    static Result resultOf(const Callable &callable)
    {
        try {
            return toResult(callable());
        } catch (const Result &result) {
            return result;
        }
    }
};

template<typename Callable>
struct ResultHelper<Callable, void>
{
    static Result resultOf(const Callable &callable)
    {
        try {
            callable();
            return Result::Success;
        } catch (const Result &result) {
            return result;
        }
    }
};

template<typename Testable>
Property<Testable>::Property(Testable testable)
        : m_quantifier(std::move(testable)) {}

template<typename Testable>
Result Property<Testable>::operator()() const
{ return ResultHelper<decltype(m_quantifier)>::resultOf(m_quantifier); }

template<typename Testable>
gen::GeneratorUP<Result> toProperty(Testable testable)
{
    return gen::GeneratorUP<Result>(new Property<Testable>(testable));
}

} // namespace detail
} // namespace rc
