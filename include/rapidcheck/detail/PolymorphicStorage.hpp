namespace rc {
namespace detail {

template<std::size_t Size>
template<typename Type, typename ...Args>
void PolymorphicStorage<Size>::init(Args &&...args)
{
    static_assert(std::is_base_of<IPolymorphic, Type>::value,
                  "Type must inherit from IPolymorphic");
    static_assert(sizeof(m_storage) >= sizeof(Type),
                  "The selected type does not fit into storage");

    new (&m_storage) Type(std::forward<Args>(args)...);
}

template<std::size_t Size>
template<typename Preferred, typename Fallback, typename ...Args>
void PolymorphicStorage<Size>::initWithFallback(Args &&...args)
{
    using Type = typename std::conditional<
        sizeof(Preferred) <= Size,
        Preferred,
        Fallback>::type;
    init<Type>(std::forward<Args>(args)...);
}

template<std::size_t Size>
template<typename T>
T &PolymorphicStorage<Size>::get()
{
    static_assert(std::is_base_of<IPolymorphic, T>::value,
                  "T must inherit from IPolymorphic");
    return *reinterpret_cast<T *>(&m_storage);
}

template<std::size_t Size>
template<typename T>
const T &PolymorphicStorage<Size>::get() const
{
    static_assert(std::is_base_of<IPolymorphic, T>::value,
                  "T must inherit from IPolymorphic");
    return *reinterpret_cast<const T *>(&m_storage);
}

template<std::size_t Size>
PolymorphicStorage<Size>::PolymorphicStorage(
    const PolymorphicStorage<Size> &other)
{
    const auto otherImpl =
        reinterpret_cast<const IPolymorphic *>(&other.m_storage);
    otherImpl->copyInto(&m_storage);
}

template<std::size_t Size>
PolymorphicStorage<Size>::PolymorphicStorage(
    PolymorphicStorage<Size> &&other)
{
    const auto otherImpl = reinterpret_cast<IPolymorphic *>(&other.m_storage);
    otherImpl->moveInto(&m_storage);
}

template<std::size_t Size>
PolymorphicStorage<Size> &
PolymorphicStorage<Size>::operator=(
    const PolymorphicStorage<Size> &rhs)
{
    const auto impl = reinterpret_cast<IPolymorphic *>(&m_storage);
    impl->~IPolymorphic();
    const auto rhsImpl = reinterpret_cast<const IPolymorphic *>(&rhs.m_storage);
    rhsImpl->copyInto(&m_storage);
    return *this;
}

template<std::size_t Size>
PolymorphicStorage<Size> &
PolymorphicStorage<Size>::operator=(
    PolymorphicStorage<Size> &&rhs)
{
    const auto impl = reinterpret_cast<IPolymorphic *>(&m_storage);
    impl->~IPolymorphic();
    const auto rhsImpl = reinterpret_cast<IPolymorphic *>(&rhs.m_storage);
    rhsImpl->moveInto(&m_storage);
    return *this;
}

template<std::size_t Size>
PolymorphicStorage<Size>::~PolymorphicStorage()
{
    const auto impl = reinterpret_cast<IPolymorphic *>(&m_storage);
    impl->~IPolymorphic();
}

} // namespace detail
} // namespace rc

#include "PolymorphicStorage.h"
