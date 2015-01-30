#pragma once

namespace rc {
namespace detail {

static inline CaseResult toCaseResult(bool value)
{
    return value
        ? CaseResult { .type = CaseResult::Type::Success,
                       .description = "returned true" }
        : CaseResult { .type = CaseResult::Type::Failure,
                       .description = "returned false" };
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
        return CaseResult { .type = CaseResult::Type::Success };
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
        return CaseResult { .type = CaseResult::Type::Discard,
                            .description = e.what() };
    } catch (const std::exception &e) {
        return CaseResult { .type = CaseResult::Type::Failure,
                            .description = e.what() };
    } catch (...) {
        return CaseResult { .type = CaseResult::Type::Failure,
                            .description = "Unknown exception thrown" };
    }
}

template<typename Testable>
Property<Testable> toProperty(const Testable &testable)
{
    return Property<Testable>(testable);
}

} // namespace detail
} // namespace rc
