#pragma once

#include <type_traits>
#include <vector>
#include <memory>

namespace rc {
namespace shrink {

//! Instances of this class are used to implement shrinking. A \c Iterator
//! type \c T successively yields possible shrinks of some value of that type
//! until all possibilities are exhausted.
template<typename T>
class Iterator
{
public:
    //! The type shrunk by this \c Iterator.
    typedef T ShrunkType;

    //! Returns \c true if this \c Iterator has more values or \c false if
    //! all possible shrinks have been exhausted.
    virtual bool hasNext() const = 0;

    //! Returns the next possible shrink value. The result of calling this
    //! method if \c hasNext returns false is undefined.
    virtual T next() = 0;

    virtual ~Iterator() = default;
};


//! \c std::unique_ptr to Iterator
template<typename T>
using IteratorUP = std::unique_ptr<Iterator<T>>;

//! Returns a \c Iterator which tries the given \c Iterators in
//! sequence.
template<typename IteratorUP, typename ...IteratorsUP>
IteratorUP sequentially(IteratorUP &&iterator, IteratorsUP &&...iterators);

//! Takes an iterate functor and a stop predicate. To yield each new value, the
//! iterate functor is called with an iterator value and yields a pair of the
//! next shrink as well as a new iterator value. This is repeated until the
//! predicate returns false for the iterator value.
//!
//! @param initial    The initial iterator value.
//! @param predicate  The stop predicate.
//! @param iterate    The iterate functor.
template<typename I,
         typename Predicate,
         typename Iterate>
IteratorUP<typename std::result_of<Iterate(I)>::type::first_type>
unfold(I initial, Predicate predicate, Iterate iterate);

//! Returns a shrink iterator which doesn't return any possible shrinks.
template<typename T>
IteratorUP<T> nothing();

//! Maps an iterator of one type to an iterator of another type.
//!
//! @param iterator  The \c Iterator to map.
//! @param mapper    The mapping function.
template<typename T, typename Mapper>
IteratorUP<typename std::result_of<Mapper(T)>::type>
map(IteratorUP<T> iterator, Mapper mapper);

//! Creates an iterator which tries the constant values in the given vector
//! in order.
template<typename T>
IteratorUP<T> constant(std::vector<T> constants);

//! Shrinks the given collection by trying to remove successively smaller chunks
//! of it.
template<typename T>
IteratorUP<T> removeChunks(T collection);

//! Tries to shrink each element of the given collection using the given
//! callable to create iterators.
//!
//! @param collection  The collection whose elements to shrink.
//! @param factory     A callable which returns an `IteratorUP<T>` when given an
//!                    element to shrink.
template<typename T, typename IteratorFactory>
IteratorUP<T> eachElement(T collection, IteratorFactory factory);

//! Shrinks an integral value towards another integral value.
//!
//! @param value   The value to shrink.
//! @param target  The integer to shrink towards.
template<typename T>
IteratorUP<T> towards(T value, T target);

//! Returns an iterator which wraps the given iterator but only yields the
//! values for which the given predicate returns true.
template<typename T, typename Predicate>
IteratorUP<T> filter(IteratorUP<T> &&iterator, Predicate predicate);

} // namespace shrink
} // namespace rc

#include "detail/Shrink.hpp"
