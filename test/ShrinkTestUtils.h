#pragma once

namespace rc {

//! Retrieves all elements from the iterator and returns them as a vector.
template<typename T>
std::vector<T> takeAll(const shrink::IteratorUP<T> &iterator)
{
    std::vector<T> items;
    while (iterator->hasNext())
        items.push_back(iterator->next());
    return items;
}

//! Returns the final element of the iterator.
template<typename T>
T finalShrink(const shrink::IteratorUP<T> &iterator)
{
    T value;
    while (iterator->hasNext())
        value = iterator->next();
    return value;
}

//! Returns the number of shrinks of the given iterator.
template<typename T>
size_t shrinkCount(const shrink::IteratorUP<T> &iterator)
{
    size_t n = 0;
    while (iterator->hasNext()) n++;
    return n;
}

} // namespace rc
