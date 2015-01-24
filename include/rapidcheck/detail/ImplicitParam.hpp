#pragma once

namespace rc {
namespace detail {

template<typename Param>
void ImplicitParam<Param>::let(ValueType value)
{
    if (m_isBinding)
        values().pop();
    values().push(std::move(value));
    m_isBinding = true;
}

template<typename Param>
typename ImplicitParam<Param>::ValueType &
ImplicitParam<Param>::operator*()
{ return values().top(); }

template<typename Param>
const typename ImplicitParam<Param>::ValueType &
ImplicitParam<Param>::operator*() const
{ return values().top(); }

template<typename Param>
typename ImplicitParam<Param>::ValueType &
ImplicitParam<Param>::operator->()
{ return **this; }

template<typename Param>
const typename ImplicitParam<Param>::ValueType &
ImplicitParam<Param>::operator->() const
{ return **this; }

template<typename Param>
ImplicitParam<Param>::~ImplicitParam()
{
    if (m_isBinding)
        values().pop();
}

template<typename Param>
typename ImplicitParam<Param>::StackT &ImplicitParam<Param>::values()
{
    // TODO needs thread local
    static StackT valueStack;
    if (valueStack.empty())
        valueStack.push(Param::defaultValue());
    return valueStack;
}

template<typename Param>
std::ostream &operator<<(std::ostream& os, const ImplicitParam<Param> &p)
{
    os << *p;
    return os;
}

} // namespace detail
} // namespace rc
