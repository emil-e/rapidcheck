#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/detail/NewProperty.h"

#include "util/Generators.h"
#include "util/Predictable.h"
#include "util/GenUtils.h"
#include "util/ShrinkableUtils.h"

using namespace rc;
using namespace rc::test;
using namespace rc::detail;

namespace {

template<typename Callable>
PropertyWrapper<Decay<Callable>> makeWrapper(Callable &&callable)
{ return PropertyWrapper<Decay<Callable>>(std::forward<Callable>(callable)); }

} // namespace

TEST_CASE("PropertyWrapper") {
    SECTION("returns success result for void callables") {
        REQUIRE(makeWrapper([]{})().type == CaseResult::Type::Success);
    }

    SECTION("if callable returns a bool") {
        SECTION("returns success result for true") {
            REQUIRE(makeWrapper([]{ return true; })().type ==
                    CaseResult::Type::Success);
        }

        SECTION("returns success result for false") {
            REQUIRE(makeWrapper([]{ return false; })().type ==
                    CaseResult::Type::Failure);
        }
    }

    SECTION("returns success for empty strings") {
        REQUIRE(makeWrapper([]{ return std::string(); })().type ==
                CaseResult::Type::Success);
    }

    newprop(
        "returns failure with the string as message for non-empty strings",
        [] {
            // TODO non-empty generator
            const auto msg = *newgen::suchThat<std::string>(
                [](const std::string &s) { return !s.empty(); });
            const auto result = makeWrapper([=]{ return msg; })();
            RC_ASSERT(result.type == CaseResult::Type::Failure);
            RC_ASSERT(result.description == msg);
        });

    newprop(
        "if a CaseResult is thrown, returns that case result",
        [](const CaseResult &result) {
            RC_ASSERT(makeWrapper([=]{ throw result; })() == result);
        });

    newprop(
        "returns a discard result if a GenerationFailure is thrown",
        [](const std::string &msg) {
            const auto result = makeWrapper([=]{
                throw GenerationFailure(msg);
            })();
            RC_ASSERT(result.type == CaseResult::Type::Discard);
            RC_ASSERT(result.description == msg);
        });

    newprop(
        "returns a failure result with what message if an std::exception is"
        " thrown",
        [](const std::string &msg) {
            const auto result = makeWrapper([=]{
                throw std::runtime_error(msg);
            })();
            RC_ASSERT(result.type == CaseResult::Type::Failure);
            RC_ASSERT(result.description == msg);
        });

    newprop(
        "returns a failure result with the string as the message if a string"
        " is thrown",
        [](const std::string &msg) {
            const auto result = makeWrapper([=]{
                throw msg;
            })();
            RC_ASSERT(result.type == CaseResult::Type::Failure);
            RC_ASSERT(result.description == msg);
        });

    SECTION("returns a failure result if other values are thrown") {
        const auto result = makeWrapper([=]{
            throw 1337;
        })();
        RC_ASSERT(result.type == CaseResult::Type::Failure);
    }

    newprop(
        "forwards arguments to callable",
        [](int a, const std::string &b, NonCopyable c) {
            const auto expected =
                std::to_string(a) + b + std::to_string(c.extra);
            const auto wrapper = makeWrapper(
                [](int a, const std::string &b, NonCopyable c) {
                    return std::to_string(a) + b + std::to_string(c.extra);
                });
            const auto result = wrapper(std::move(a),
                                        std::move(b),
                                        std::move(c));
            RC_ASSERT(result.description == expected);
        });
}

namespace {

template<int N>
struct Fixed {};

template<int N>
void showValue(std::ostream &os, const Fixed<N> &) { os << N; }

} // namespace

namespace rc {

template<int N>
struct NewArbitrary<Fixed<N>>
{
    static Gen<Fixed<N>> arbitrary()
    {
        return [](const Random &random, int) {
            const int n = Random(random).next() % 10;
            return shrinkable::just(
                Fixed<N>(),
                seq::take(n, seq::repeat(shrinkable::just(Fixed<N>()))));
        };
    }
};

} // namespace rc

TEST_CASE("toNewProperty") {
    using ShrinkableResult = Shrinkable<CaseDescription>;

    newprop(
        "counterexample contains descriptions of picked values",
        [](const GenParams &params) {
            const auto n = *newgen::inRange<std::size_t>(0, 10);
            const auto gen = toNewProperty(
                [=](Fixed<1>, Fixed<2>, Fixed<3>) {
                    for (std::size_t i = 0; i < n; i++)
                        *newgen::arbitrary<Fixed<1337>>();
                });
            const auto shrinkable = gen(params.random, params.size);

            Example expected;
            expected.reserve(n + 1);
            using ArgsTuple = std::tuple<Fixed<1>, Fixed<2>, Fixed<3>>;
            expected.emplace_back(
                typeToString<ArgsTuple>(),
                toString(std::make_tuple(Fixed<1>(), Fixed<2>(), Fixed<3>())));
            expected.insert(end(expected), n,
                            std::make_pair(typeToString<Fixed<1337>>(),
                                           toString(Fixed<1337>())));

            onAnyPath(
                shrinkable,
                [&](const ShrinkableResult &value,
                    const ShrinkableResult &shrink)
                {
                    RC_ASSERT(value.value().example == expected);
                });
        });

    newprop(
        "case result corresponds to counter example",
        [](const GenParams &params) {
            const auto gen = toNewProperty(
                [=] {
                    return (*newgen::arbitrary<int>() % 2) == 0;
                });
            const auto shrinkable = gen(params.random, params.size);

            onAnyPath(
                shrinkable,
                [](const ShrinkableResult &value,
                   const ShrinkableResult &shrink) {
                    const auto desc = value.value();
                    RC_ASSERT(
                        (desc.result.type == CaseResult::Type::Success) ==
                        ((std::stoi(desc.example.back().second) % 2) == 0));
                });
        });
}
