#pragma once

#include "rapidcheck/Seq.h"

namespace rc {
namespace newshrink {

//! Shrinks the given collection by trying to remove successively smaller chunks
//! of it.
template<typename T>
Seq<T> removeChunks(T collection);

//! Tries to shrink each element of the given collection using the given
//! callable to create sequences shrinks for that element.
//!
//! @param collection     The collection whose elements to shrink.
//! @param shrinkElement  A callable which returns a `Seq<T>` given an element
//!                       to shrink.
template<typename T, typename ShrinkElement>
Seq<T> eachElement(T collection, ShrinkElement shrinkElement);

//! Shrinks an integral value towards another integral value.
//!
//! @param value   The value to shrink.
//! @param target  The integer to shrink towards.
template<typename T>
Seq<T> towards(T value, T target);

} // namespace newshrink
} // namespace rc

#include "NewShrink.hpp"
