#pragma once

#include <stdexcept>

namespace rc {
namespace test {

//! Test utility to ensure that no copies take place
struct CopyGuard
{
    CopyGuard(int x = 0) : value(x) {}

    CopyGuard(const CopyGuard &)
    { throw std::runtime_error("CopyGuard was copied"); }

    CopyGuard &operator=(const CopyGuard &)
    { throw std::runtime_error("CopyGuard was copied"); }

    CopyGuard(CopyGuard &&) = default;
    CopyGuard &operator=(CopyGuard &&) = default;

    int value;
};

void showValue(CopyGuard value, std::ostream &os)
{ os << "[CopyGuard]"; }


static inline bool operator==(const CopyGuard &lhs, const CopyGuard &rhs)
{ return lhs.value == rhs.value; }

static inline bool operator<(const CopyGuard &lhs, const CopyGuard &rhs)
{ return lhs.value < rhs.value; }

} // namespace test

template<>
class Arbitrary<test::CopyGuard> : public gen::Generator<test::CopyGuard>
{
public:
    test::CopyGuard generate() const override
    { return test::CopyGuard(*gen::arbitrary<int>()); }
};

} // namespace rc

namespace std {

template<>
struct hash<rc::test::CopyGuard>
{
    typedef rc::test::CopyGuard argument_type;
    typedef std::size_t value_type;

    value_type operator()(const rc::test::CopyGuard &value) const
    { return std::hash<int>()(value.value); }
};

}
