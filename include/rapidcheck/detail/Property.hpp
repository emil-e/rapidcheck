#pragma once

namespace rc {
namespace detail {

static inline CaseResult toCaseResult(bool value)
{
    return value
        ? CaseResult(CaseResult::Type::Success)
        : CaseResult(CaseResult::Type::Failure);
}

//! Helper class to convert different return types to `CaseResult`.
template<typename Callable,
         typename ReturnType = typename std::result_of<Callable()>::type>
struct CaseResultHelper
{
    static CaseResult resultOf(const Callable &callable)
    { return toCaseResult(callable()); }
};

template<typename Callable>
struct CaseResultHelper<Callable, void>
{
    static CaseResult resultOf(const Callable &callable)
    {
        callable();
        return CaseResult(CaseResult::Type::Success);
    }
};

template<typename Testable>
Property<Testable>::Property(Testable testable)
        : m_quantifier(std::move(testable)) {}

template<typename Testable>
CaseResult Property<Testable>::operator()() const
{
    try {
        return CaseResultHelper<decltype(m_quantifier)>::resultOf(m_quantifier);
    } catch (const CaseResult &result) {
        return result;
    } catch (const std::exception &e) {
        return CaseResult(CaseResult::Type::Failure, e.what());
    } catch (...) {
        return CaseResult(CaseResult::Type::Failure, "Unknown exception thrown");
    }
}

template<typename Testable>
gen::GeneratorUP<CaseResult> toProperty(Testable testable)
{
    return gen::GeneratorUP<CaseResult>(new Property<Testable>(testable));
}

} // namespace detail
} // namespace rc
