#pragma once

namespace rc {

#define RC_SIGNED_INTEGRAL_TYPES                \
    char,                                       \
    short,                                      \
    int,                                        \
    long,                                       \
    long long

#define RC_UNSIGNED_INTEGRAL_TYPES              \
    unsigned char,                              \
    unsigned short,                             \
    unsigned int,                               \
    unsigned long,                              \
    unsigned long long

#define RC_INTEGRAL_TYPES RC_SIGNED_INTEGRAL_TYPES, RC_UNSIGNED_INTEGRAL_TYPES

#define RC_REAL_TYPES double, float

#define RC_SIGNED_TYPES RC_REAL_TYPES, RC_SIGNED_INTEGRAL_TYPES

#define RC_NUMERIC_TYPES RC_REAL_TYPES, RC_INTEGRAL_TYPES

#define RC_GENERIC_CONTAINERS(T)                            \
    std::vector<T>,                                         \
    std::deque<T>,                                          \
    std::forward_list<T>,                                   \
    std::list<T>,                                           \
    std::set<T>,                                            \
    std::map<T, T>,                                         \
    std::multiset<T>,                                       \
    std::multimap<T, T>,                                    \
    std::unordered_set<T>,                                  \
    std::unordered_map<T, T>,                               \
    std::unordered_multiset<T>,                             \
    std::unordered_multimap<T, T>

template<typename T, typename Testable>
void templatedProp(const std::string &description, Testable testable)
{
    prop(description + " (" + detail::demangle(typeid(T).name()) + ")",
         testable);
}


#define TEMPLATED_SECTION(tparam, description)  \
    SECTION(std::string(description) + " (" +   \
            detail::demangle(typeid(T).name()) + ")")


template<typename Callable>
auto testEnv(const Callable &callable) -> decltype(callable())
{
    using namespace detail;

    ImplicitParam<param::Size> size;
    if (!size.hasBinding())
        size.let(0);

    ImplicitParam<param::NoShrink> noShrink;

    ImplicitParam<param::RandomEngine> randomEngine;
    randomEngine.let(RandomEngine());
    randomEngine->seed(pick<RandomEngine::Atom>());

    ImplicitParam<param::CurrentNode> currentNode;
    currentNode.let(nullptr);

    return callable();
}

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

//! Returns true if there is a shrink in the given shrink iterator that
//! satisfies the given predicate.
template<typename T, typename Predicate>
bool hasShrinkSuchThat(const shrink::IteratorUP<T> &iterator,
                       Predicate predicate)
{
    while (iterator->hasNext()) {
        if (predicate(iterator->next()))
            return true;
    }

    return false;
}

//! So that we in templated tests can compare map pairs with their non-const-key
//! equivalents.
template<typename T1, typename T2>
bool operator==(const std::pair<const T1, T2> &lhs,
                const std::pair<T1, T2> &rhs)
{
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

//! Returns the size of the given container by counting them through iterators.
template<typename T>
typename T::size_type containerSize(const T &container) {
    typename T::size_type size = 0;
    for (auto it = begin(container); it != end(container); it++)
        size++;
    return size;
}

} // namespace rc
