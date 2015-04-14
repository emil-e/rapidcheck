#pragma once

namespace rc {
namespace detail {

//! Base class for objects stored in `PolymorphicStorage`. You must inherit from
//! this if you want to use `PolymorphicStorage`.
class IPolymorphic
{
public:
    //! Copy this object into the storage pointed to by `storage`.
    virtual void copyInto(void *storage) const = 0;

    //! Move this object into the storage pointed to by `storage`.
    virtual void moveInto(void *storage) = 0;

    virtual ~IPolymorphic() = default;
};

//! Helper class to implement small-object optimization for common types.
//! `Interface` must implement `void copyInto(void *) const` and
//! `void moveInto(void *)`.
//!
//! NOTE: You MUST call `init` before the object is used or even destroyed. This
//! class does not support uninitialized storage. The constructor is not used
//! since constructors cannot have explicitly specified type parameters.
template<std::size_t Size>
class PolymorphicStorage
{
public:
    //! You MUST call `init` after this.
    PolymorphicStorage() = default;

    //! The storage size.
    static constexpr auto size = Size;

    //! Initializes the storage with the given type. The object will be
    //! constructed using the given arguments.
    //!
    //! WARNING: This function is dangerous. You MUST call this before use. In
    //! addition, make sure to call it only _once_ since there are no checks
    //! performed to see that it is used properly.
    template<typename Type, typename ...Args>
    void init(Args &&...args);

    //! Initializes the storage with one of the given types. If `Preferred` is
    //! small enough to fit into the storage, that is used, otherwise `Fallback`
    //! will be used. Both objects will be constructed using the given
    //! arguments.
    //!
    //! WARNING: See `init` for more info about the dangers.
    template<typename Preferred, typename Fallback, typename ...Args>
    void initWithFallback(Args &&...args);

    //! Retrieves a reference of type `T` to the contained object.
    template<typename T>
    T &get();

    //! Retrieves a const-reference of type `T` to the contained object.
    template<typename T>
    const T &get() const;

    PolymorphicStorage(const PolymorphicStorage &other);
    PolymorphicStorage(PolymorphicStorage &&other);
    PolymorphicStorage &operator=(const PolymorphicStorage &rhs);
    PolymorphicStorage &operator=(PolymorphicStorage &&rhs);

    ~PolymorphicStorage();

private:
    typedef typename std::aligned_storage<Size>::type Storage;
    Storage m_storage;
};

} // namespace detail
} // namespace rc

#include "PolymorphicStorage.hpp"
