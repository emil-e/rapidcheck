#pragma once

namespace rc {
namespace detail {

template<typename ...Types>
struct IndexHelper;

template<>
struct IndexHelper<>
{
    template<typename T>
    static constexpr std::size_t indexOf() { return 0; }
};

template<typename First, typename ...Types>
struct IndexHelper<First, Types...>
{
    template<typename T>
    static constexpr std::size_t indexOf()
    {
        return std::is_same<First, T>::value
            ? 0
            : IndexHelper<Types...>::template indexOf<T>() + 1;
    }
};

template<typename ...Types>
template<typename T, typename>
Variant<Types...>::Variant(T &&value)
    noexcept(std::is_nothrow_constructible<Decay<T>, T &&>::value)
    : m_typeIndex(indexOfType<Decay<T>>())
{
    static_assert(isValidType<Decay<T>>(),
                  "T is not a valid type of this variant");

    new (&m_storage) Decay<T>(std::forward<T>(value));
}

template<typename ...Types>
template<typename T, typename>
Variant<Types...> &Variant<Types...>::operator=(const T &value) noexcept
{
    static_assert(isValidType<T>(),
                  "T is not a valid type of this variant");

    const auto newIndex = indexOfType<T>();
    if (newIndex == m_typeIndex) {
        *reinterpret_cast<T *>(&m_storage) = value;
    } else {
        destroy(m_typeIndex, &m_storage);
        m_typeIndex = newIndex;
        new (&m_storage) T(value);
    }
    return *this;
}

template<typename ...Types>
template<typename T, typename>
Variant<Types...> &Variant<Types...>::operator=(T &&value) noexcept
{
    static_assert(isValidType<T>(),
                  "T is not a valid type of this variant");

    const auto newIndex = indexOfType<T>();
    if (newIndex == m_typeIndex) {
        *reinterpret_cast<T *>(&m_storage) = std::move(value);
    } else {
        destroy(m_typeIndex, &m_storage);
        m_typeIndex = newIndex;
        new (&m_storage) T(std::move(value));
    }
    return *this;
}

template<typename ...Types>
template<typename T>
T &Variant<Types...>::get()
{
    assert(indexOfType<T>() == m_typeIndex);
    return *reinterpret_cast<T *>(&m_storage);
}

template<typename ...Types>
template<typename T>
const T &Variant<Types...>::get() const
{
    assert(indexOfType<T>() == m_typeIndex);
    return *reinterpret_cast<const T *>(&m_storage);
}

// TODO this would be more fun with varargs and lambdas
template<typename ...Types>
template<typename T>
bool Variant<Types...>::match(T &value) const
{
    if (!is<T>())
        return false;

    value = *reinterpret_cast<const T *>(&m_storage);
    return true;
}

template<typename ...Types>
template<typename T>
bool Variant<Types...>::is() const
{
    static_assert(isValidType<Decay<T>>(),
                  "T is not a valid type of this variant");
    return m_typeIndex == indexOfType<T>();
}

template<typename T>
bool variantEqualsImpl(const void *lhs, const void *rhs)
{ return *static_cast<const T *>(lhs) == *static_cast<const T *>(rhs); }

template<typename ...Types>
bool Variant<Types...>::operator==(const Variant &rhs) const
{
    if (m_typeIndex != rhs.m_typeIndex)
        return false;

    bool (* const equalsFuncs[])(const void *, const void *) = {
        &variantEqualsImpl<Types>...
    };

    return equalsFuncs[m_typeIndex](&m_storage, &rhs.m_storage);
}

template<typename T>
void variantPrintToImpl(std::ostream &os, const void *storage)
{ os << *static_cast<const T *>(storage); }

template<typename ...Types>
void Variant<Types...>::printTo(std::ostream &os) const
{
    void (*printToFuncs[])(std::ostream &, const void *) = {
        &variantPrintToImpl<Types>...
    };

    printToFuncs[m_typeIndex](os, &m_storage);
}

template<typename ...Types>
Variant<Types...>::Variant(const Variant &other)
    noexcept(All<std::is_nothrow_copy_constructible, Types...>::value)
    : m_typeIndex(other.m_typeIndex)
{ copy(m_typeIndex, &m_storage, &other.m_storage); }

template<typename ...Types>
Variant<Types...>::Variant(Variant &&other)
    noexcept(All<std::is_nothrow_move_constructible, Types...>::value)
    : m_typeIndex(other.m_typeIndex)
{ move(m_typeIndex, &m_storage, &other.m_storage); }

template<typename ...Types>
Variant<Types...> &Variant<Types...>::operator=(const Variant &rhs)
    noexcept(All<std::is_nothrow_copy_constructible, Types...>::value &&
             All<std::is_nothrow_copy_assignable, Types...>::value)
{
    static_assert(
        All<std::is_nothrow_move_constructible, Types...>::value,
        "All types must be nothrow move-constructible to use copy assignment");

    if (m_typeIndex == rhs.m_typeIndex) {
        copyAssign(m_typeIndex, &m_storage, &rhs.m_storage);
    } else {
        Storage tmp;
        copy(rhs.m_typeIndex, &tmp, &rhs.m_storage);
        destroy(m_typeIndex, &m_storage);
        move(rhs.m_typeIndex, &m_storage, &tmp);
        m_typeIndex = rhs.m_typeIndex;
    }

    return *this;
}

template<typename ...Types>
Variant<Types...> &Variant<Types...>::operator=(Variant &&rhs)
    noexcept(All<std::is_nothrow_move_assignable, Types...>::value)
{
    static_assert(
        All<std::is_nothrow_move_constructible, Types...>::value,
        "All types must be nothrow move-constructible to use copy assignment");

    if (m_typeIndex == rhs.m_typeIndex) {
        moveAssign(m_typeIndex, &m_storage, &rhs.m_storage);
    } else {
        destroy(m_typeIndex, &m_storage);
        m_typeIndex = rhs.m_typeIndex;
        move(m_typeIndex, &m_storage, &rhs.m_storage);
    }

    return *this;
}

template<typename ...Types>
Variant<Types...>::~Variant() noexcept { destroy(m_typeIndex, &m_storage); }

template<typename T>
void variantCopyAssign(void *to, const void *from)
{ *static_cast<T *>(to) = *static_cast<const T *>(from); }

template<typename ...Types>
void Variant<Types...>::copyAssign(
    std::size_t index, void *to, const void *from)
{
    static void (* const copyAssignFuncs[])(void *, const void *) = {
        &variantCopyAssign<Types>...
    };

    copyAssignFuncs[index](to, from);
}

template<typename T>
void variantMoveAssign(void *to, void *from)
{ *static_cast<T *>(to) = std::move(*static_cast<T *>(from)); }

template<typename ...Types>
void Variant<Types...>::moveAssign(std::size_t index, void *to, void *from)
{
    static void (* const moveAssignFuncs[])(void *, void *) = {
        &variantMoveAssign<Types>...
    };

    moveAssignFuncs[index](to, from);
}

template<typename T>
void variantCopy(void *to, const void *from)
{ new (to) T(*static_cast<const T *>(from)); }

template<typename ...Types>
void Variant<Types...>::copy(
    std::size_t index, void *to, const void *from)
{
    static void (* const copyFuncs[])(void *, const void *) = {
        &variantCopy<Types>...
    };

    copyFuncs[index](to, from);
}

template<typename T>
void variantMove(void *to, void *from)
{ new (to) T(std::move(*static_cast<T *>(from))); }

template<typename ...Types>
void Variant<Types...>::move(std::size_t index, void *to, void *from)
{
    static void (* const moveFuncs[])(void *, void *) = {
        &variantMove<Types>...
    };

    moveFuncs[index](to, from);
}

template<typename T>
void variantDestroy(void *storage) { static_cast<T *>(storage)->~T(); }

template<typename ...Types>
void Variant<Types...>::destroy(std::size_t index, void *storage) noexcept
{
    static void (* const destroyFuncs[])(void *) = {
        &variantDestroy<Types>...
    };

    destroyFuncs[index](storage);
}

template<typename ...Types>
template<typename T>
constexpr std::size_t Variant<Types...>::indexOfType()
{ return IndexHelper<Types...>::template indexOf<T>(); }

template<typename ...Types>
template<typename T>
constexpr bool Variant<Types...>::isValidType()
{ return indexOfType<T>() < sizeof...(Types); }

template<typename ...Types>
bool operator!=(const Variant<Types...> &lhs, const Variant<Types...> &rhs)
{ return !(lhs == rhs); }

} // namespace detail
} // namespace rc
