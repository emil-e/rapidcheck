#pragma once

namespace rc {

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

inline bool operator<(const MyNonCopyable &lhs, const MyNonCopyable &rhs)
{ return lhs.value < rhs.value; }

inline bool operator==(const MyNonCopyable &lhs, const MyNonCopyable &rhs)
{ return lhs.value == rhs.value; }

template<>
class Arbitrary<MyNonCopyable> : public gen::Generator<MyNonCopyable>
{
public:
    MyNonCopyable generate() const override
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

namespace std {

template<>
struct hash<rc::MyNonCopyable>
{
    typedef rc::MyNonCopyable argument_type;
    typedef std::size_t value_type;

    value_type operator()(const rc::MyNonCopyable &value) const
    { return std::hash<int>()(value.value); }
};

} // namespace std
