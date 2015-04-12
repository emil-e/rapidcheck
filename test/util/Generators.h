#pragma once

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Seq.h"
#include "rapidcheck/Shrinkable.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/Maybe.h"
#include "rapidcheck/seq/Create.h"

namespace rc {

template<>
struct NewArbitrary<detail::Configuration>
{
    static Gen<detail::Configuration> arbitrary()
    {
        return newgen::exec([]{
            detail::Configuration config;
            config.seed = *newgen::arbitrary<uint64_t>();
            config.maxSuccess = *newgen::inRange<int>(0, 1000);
            config.maxSize = *newgen::inRange<int>(0, 1000);
            config.maxDiscardRatio = *newgen::inRange<int>(0, 100);
            return config;
        });
    }
};

template<>
struct NewArbitrary<detail::TestCase>
{
    static Gen<detail::TestCase> arbitrary()
    {
        return newgen::exec([]{
            detail::TestCase testCase;
            testCase.size = *newgen::withSize([](int size) {
                // TODO this should be replaced by a sized ranged generator
                // instead
                return newgen::inRange<int>(0, size + 1);
            });
            testCase.seed = *newgen::arbitrary<decltype(testCase.seed)>();
            return testCase;
        });
    }
};

template<>
struct NewArbitrary<detail::CaseResult::Type>
{
    static Gen<detail::CaseResult::Type> arbitrary()
    {
        return newgen::element(
            detail::CaseResult::Type::Success,
            detail::CaseResult::Type::Failure,
            detail::CaseResult::Type::Discard);
    }
};

template<>
struct NewArbitrary<detail::CaseResult>
{
    static Gen<detail::CaseResult> arbitrary()
    {
        return newgen::exec([]{
            detail::CaseResult result;
            result.type = *newgen::arbitrary<detail::CaseResult::Type>();
            result.description = *newgen::arbitrary<std::string>();
            return result;
        });
    }
};

template<>
struct NewArbitrary<detail::SuccessResult>
{
    static Gen<detail::SuccessResult> arbitrary()
    {
        return newgen::map(newgen::positive<int>(), [](int s) {
            detail::SuccessResult result;
            result.numSuccess = s;
            return result;
        });
    }
};

template<>
struct NewArbitrary<detail::FailureResult>
{
    static Gen<detail::FailureResult> arbitrary()
    {
        return newgen::exec([]{
            detail::FailureResult result;
            result.numSuccess = *newgen::positive<int>();
            result.failingCase = *newgen::arbitrary<detail::TestCase>();
            result.description = *newgen::arbitrary<std::string>();
            result.numShrinks = *newgen::positive<int>();
            result.counterExample = *newgen::arbitrary<
                decltype(result.counterExample)>();
            return result;
        });
    }
};

template<>
struct NewArbitrary<detail::GaveUpResult>
{
    static Gen<detail::GaveUpResult> arbitrary()
    {
        return newgen::exec([]{
            detail::GaveUpResult result;
            result.numSuccess = *newgen::positive<int>();
            result.description = *newgen::arbitrary<std::string>();
            return result;
        });
    }
};

template<typename T>
struct NewArbitrary<Seq<T>>
{
    static Gen<Seq<T>> arbitrary()
    {
        return newgen::map<std::vector<T>>(
            &seq::fromContainer<std::vector<T>>);
    }
};

template<typename T>
inline Shrinkable<Maybe<T>> prependNothing(Shrinkable<Maybe<T>> &&s)
{
    return shrinkable::mapShrinks(
        std::move(s),
        [](Seq<Shrinkable<Maybe<T>>> &&shrinks) {
            return seq::concat(
                seq::just(shrinkable::just(Maybe<T>())),
                seq::map(std::move(shrinks), &prependNothing<T>));
        });
};

template<typename T>
struct NewArbitrary<Maybe<T>>
{
    static Gen<Maybe<T>> arbitrary()
    {
        return [](const Random &random, int size) {
            auto r = random;
            const auto x = r.split().next() % (size + 1);
            if (x == 0)
                return shrinkable::just(Maybe<T>());

            return prependNothing(
                shrinkable::map(
                    newgen::arbitrary<T>()(r, size),
                    [](T &&x) -> Maybe<T> { return std::move(x); }));
        };
    }
};

template <typename T>
struct NewArbitrary<Shrinkable<T>>
{
    static Gen<Shrinkable<T>> arbitrary()
    {
        // TODO fapply
        return newgen::map(
            newgen::tuple(
                newgen::arbitrary<T>(),
                newgen::scale(
                    0.25,
                    newgen::lazy(&newgen::arbitrary<Seq<Shrinkable<T>>>))),
            [](std::pair<T, Seq<Shrinkable<T>>> &&p) {
                return shrinkable::just(std::move(p.first),
                                        std::move(p.second));
            });
    }
};

} // namespace rc
