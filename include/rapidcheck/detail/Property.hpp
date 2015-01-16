#pragma once

namespace rc {
namespace detail {

static inline CaseResult toCaseResult(bool value)
{
    return value
        ? CaseResult(CaseResult::Type::Success, "returned true")
        : CaseResult(CaseResult::Type::Failure, "returned false");
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
CaseResult Property<Testable>::generate() const
{
    try {
        return CaseResultHelper<decltype(m_quantifier)>::resultOf(m_quantifier);
    } catch (const CaseResult &result) {
        return result;
    } catch (const gen::GenerationFailure &e) {
        return CaseResult(CaseResult::Type::Discard, e.what());
    } catch (const std::exception &e) {
        return CaseResult(CaseResult::Type::Failure, e.what());
    } catch (...) {
        return CaseResult(CaseResult::Type::Failure, "Unknown exception thrown");
    }
}

template<typename Testable>
Property<Testable> toProperty(Testable testable)
{
    return Property<Testable>(testable);
}

} // namespace detail
} // namespace rc
