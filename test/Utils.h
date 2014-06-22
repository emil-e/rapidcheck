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

struct MyNonCopyable
{
    static constexpr int genValue = 1337;
    int value;

    MyNonCopyable() = default;
    MyNonCopyable(const MyNonCopyable &) = delete;
    MyNonCopyable &operator=(const MyNonCopyable &) = delete;
    MyNonCopyable(MyNonCopyable &&) = default;
    MyNonCopyable &operator=(MyNonCopyable &&) = default;
};

template<>
class Arbitrary<MyNonCopyable> : public gen::Generator<MyNonCopyable>
{
public:
    MyNonCopyable operator()() const override
    {
        MyNonCopyable x;
        x.value = MyNonCopyable::genValue;
        return x;
    }
};

inline void show(const MyNonCopyable &x, std::ostream &os)
{
    os << x.value;
}

} // namespace rc
