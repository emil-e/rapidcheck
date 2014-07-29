#pragma once

namespace rc {

//! Useful utility class for testing that `Arbitrary` is actually used. The key
//! here is that if the default constructor is used, the value will be undefined
//! but if `Arbitrary` is used, the value will be `X`. An extra member is
//! included so that multiple unique values of the same type can be generated.
struct Predictable
{
    static constexpr int predictableValue = 1337;
    int value;
    int extra;
};

//! Non-copyable version of `Predictable`.
struct NonCopyable : public Predictable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = default;
    NonCopyable &operator=(NonCopyable &&) = default;
};

// TODO source file!

static inline bool operator<(const Predictable &lhs, const Predictable &rhs)
{
    if (lhs.value == rhs.value)
        return lhs.extra < rhs.extra;

    return lhs.value < rhs.value;
}

static inline bool operator==(const Predictable &lhs, const Predictable &rhs)
{
    return
        (lhs.value == rhs.value) &&
        (lhs.extra == rhs.extra);
}

static inline void show(const Predictable &value, std::ostream &os)
{
    show(value.value, os);
    os << " (" << value.extra << ")";
}

static inline void show(const NonCopyable &value, std::ostream &os)
{
    show(value.value, os);
    os << " (" << value.extra << ")";
}

template<>
class Arbitrary<Predictable> : public gen::Generator<Predictable>
{
public:
    Predictable generate() const override
    {
        return Predictable {
            .value = Predictable::predictableValue,
            .extra = gen::arbitrary<int>().generate() };
    }
};

template<>
class Arbitrary<NonCopyable> : public gen::Generator<NonCopyable>
{
public:
    NonCopyable generate() const override
    {
        NonCopyable value;
        value.value = Predictable::predictableValue;
        value.extra = gen::arbitrary<int>().generate();
        return value;
    }
};

// These overloads are useful to test if a value was generated using the
// appropriate `Arbitrary` specialization.

static inline bool isArbitraryPredictable(const Predictable &value)
{ return value.value == Predictable::predictableValue; }

static inline bool isArbitraryPredictable(
    const std::pair<const Predictable, Predictable> &value)
{
    return
        isArbitraryPredictable(value.first) &&
        isArbitraryPredictable(value.second);
}

} // namespace rc

namespace std {

template<>
struct hash<rc::Predictable>
{
    typedef rc::Predictable argument_type;
    typedef std::size_t value_type;

    value_type operator()(const rc::Predictable &value) const
    { return std::hash<int>()(value.value) ^ std::hash<int>()(value.extra); }
};

template<>
struct hash<rc::NonCopyable>
{
    typedef rc::NonCopyable argument_type;
    typedef std::size_t value_type;

    value_type operator()(const rc::NonCopyable &value) const
    { return std::hash<rc::Predictable>()(value); }
};

}
