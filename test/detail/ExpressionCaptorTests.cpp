#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/ExpressionCaptor.h"

using namespace rc;
using namespace rc::detail;

namespace {

struct Box
{
    Box(int x) : value(x) {}
    int value;

    std::string str() const { return "[" + std::to_string(value) + "]"; }
};

void showValue(const Box &value, std::ostream &os)
{
    os << value.str();
}

} // namespace

namespace rc {

template<>
class Arbitrary<ExpressionCaptor> : public gen::Generator<ExpressionCaptor>
{
public:
    ExpressionCaptor generate() const override
    { return ExpressionCaptor(*gen::arbitrary<std::string>()); }
};

template<>
class Arbitrary<Box> : public gen::Generator<Box>
{
public:
    Box generate() const override { return Box(*gen::arbitrary<int>()); }
};

} // namespace rc

TEST_CASE("ExpressionCaptor") {
    SECTION("str") {
        prop("returns the current value",
             [] (const std::string &value) {
                 RC_ASSERT(ExpressionCaptor(value).str() == value);
             });
    }

    SECTION("operator->*") {
        prop("simply appends RHS",
             [](const ExpressionCaptor &captor, Box value) {
                 RC_ASSERT((std::move(captor) ->* value).str() ==
                           (captor.str() + value.str()));
            });
    }

#define TEST_BINARY_OPERATOR(op)                                        \
    SECTION("operator" op) {                                            \
        prop("joins LHS and RHS with operator string between",          \
             [](const ExpressionCaptor &captor, Box value) {            \
                 RC_ASSERT((std::move(captor) op value).str() ==        \
                           (captor.str() + " " #op " " + value.str())); \
             });                                                        \
    }

    TEST_BINARY_OPERATOR(*)
    TEST_BINARY_OPERATOR(/)
    TEST_BINARY_OPERATOR(%)

    TEST_BINARY_OPERATOR(+)
    TEST_BINARY_OPERATOR(-)

    TEST_BINARY_OPERATOR(<<)
    TEST_BINARY_OPERATOR(>>)

    TEST_BINARY_OPERATOR(<)
    TEST_BINARY_OPERATOR(>)
    TEST_BINARY_OPERATOR(>=)
    TEST_BINARY_OPERATOR(<=)

    TEST_BINARY_OPERATOR(==)
    TEST_BINARY_OPERATOR(!=)

    TEST_BINARY_OPERATOR(&)

    TEST_BINARY_OPERATOR(^)

    TEST_BINARY_OPERATOR(|)

    TEST_BINARY_OPERATOR(&&)

    TEST_BINARY_OPERATOR(||)

#undef TEST_BINARY_OPERATOR
}
