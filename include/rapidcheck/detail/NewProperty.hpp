#pragma once

#include "rapidcheck/detail/FunctionTraits.h"
#include "rapidcheck/newgen/detail/ExecRaw.h"

namespace rc {
namespace detail {

std::ostream &operator<<(std::ostream &os, const CaseDescription &desc)
{
    os << desc.result << std::endl;
    os << std::endl;
    for (const auto &p : desc.example)
        os << p.first << ": " << p.second << std::endl;
    os << std::endl;
    return os;
}

static inline CaseResult newToCaseResult(bool value)
{
    return value
        ? CaseResult(CaseResult::Type::Success, "returned true")
        : CaseResult(CaseResult::Type::Failure, "returned false");
}

static inline CaseResult newToCaseResult(std::string value)
{
    return value.empty()
        ? CaseResult(CaseResult::Type::Success, "empty string")
        : CaseResult(CaseResult::Type::Failure, std::move(value));
}

//! Helper class to convert different return types to `CaseResult`.
template<typename ReturnType>
struct NewCaseResultHelper
{
    template<typename Callable, typename ...Args>
    static CaseResult resultOf(const Callable &callable, Args &&...args)
    { return newToCaseResult(callable(std::forward<Args>(args)...)); }
};

template<>
struct NewCaseResultHelper<void>
{
    template<typename Callable, typename ...Args>
    static CaseResult resultOf(const Callable &callable, Args &&...args)
    {
        callable(std::forward<Args>(args)...);
        return CaseResult(CaseResult::Type::Success, "no exceptions thrown");
    }
};

template<typename Callable,
         typename Type = FunctionType<Callable>>
class PropertyWrapper;

template<typename Callable, typename ReturnType, typename ...Args>
class PropertyWrapper<Callable, ReturnType(Args...)>
{
public:
    template<typename Arg,
             typename = typename std::enable_if<
                 !std::is_same<Decay<Arg>, PropertyWrapper>::value>::type>
    PropertyWrapper(Arg &&callable)
        : m_callable(std::forward<Arg>(callable)) {}

    CaseResult operator()(Args &&...args) const
    {
        try {
            return NewCaseResultHelper<ReturnType>::resultOf(
                m_callable, static_cast<Args &&>(args)...);
        } catch (const CaseResult &result) {
            return result;
        } catch (const GenerationFailure &e) {
            return CaseResult(CaseResult::Type::Discard, e.what());
        } catch (const std::exception &e) {
            // TODO say that it was an exception
            return CaseResult(CaseResult::Type::Failure, e.what());
        } catch (const std::string &str) {
            return CaseResult(CaseResult::Type::Failure, str);
        } catch (...) {
            return CaseResult(CaseResult::Type::Failure,
                              "Unknown object thrown");
        }
    }

private:
    Callable m_callable;
};

Gen<CaseDescription> mapToCaseDescription(
    Gen<std::pair<CaseResult, newgen::detail::Recipe>> gen);

template<typename Callable>
NewProperty toNewProperty(Callable &&callable)
{
    using Wrapper = PropertyWrapper<Decay<Callable>>;
    return mapToCaseDescription(
        newgen::detail::execRaw(
            Wrapper(std::forward<Callable>(callable))));
}

} // namespace detail
} // namespace rc

#include "NewProperty.hpp"
