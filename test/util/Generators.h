#pragma once

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Seq.h"
#include "rapidcheck/Maybe.h"
#include "rapidcheck/seq/Create.h"

namespace rc {

template<typename T, typename VariantT>
class VariantHelperGen : public gen::Generator<VariantT>
{
public:
    VariantT generate() const override
    { return VariantT(*gen::arbitrary<T>()); }
};

template<typename ...Types>
class Arbitrary<detail::Variant<Types...>>
    : public gen::Generator<detail::Variant<Types...>>
{
    detail::Variant<Types...> generate() const override
    {
        // Here's how I'd like to do it but GCC won't let me:
        //
        // return *gen::oneOf(
        //     gen::map(gen::arbitrary<Types>(),
        //              [] (Types &&x) {
        //                  return detail::Variant<Types...>(x);
        //              })...
        //     );
        //
        // So here we go instead:
        return *gen::oneOf(
            VariantHelperGen<Types, detail::Variant<Types...>>()...);
    }
};

template<>
class Arbitrary<detail::Configuration>
    : public gen::Generator<detail::Configuration>
{
public:
    detail::Configuration generate() const override
    {
        detail::Configuration config;
        config.seed = *gen::arbitrary<detail::RandomEngine::Seed>();
        config.maxSuccess = *gen::ranged<int>(0, 1000);
        config.maxSize = *gen::ranged<int>(0, 1000);
        config.maxDiscardRatio = *gen::ranged<int>(0, 100);
        return config;
    }
};

template<>
class Arbitrary<detail::TestCase> : public gen::Generator<detail::TestCase>
{
public:
    detail::TestCase generate() const override
    {
        detail::TestCase testCase;
        testCase.size = *gen::ranged<int>(0, gen::currentSize());
        testCase.seed = *gen::arbitrary<decltype(testCase.seed)>();
        return testCase;
    }
};

template<>
class Arbitrary<detail::CaseResult::Type>
    : public gen::Generator<detail::CaseResult::Type>
{
public:
    detail::CaseResult::Type generate() const override
    {
        return static_cast<detail::CaseResult::Type>(
            *gen::noShrink(
                gen::ranged<int>(0, 3)));
    }
};

template<>
class Arbitrary<detail::CaseResult> : public gen::Generator<detail::CaseResult>
{
public:
    detail::CaseResult generate() const override
    {
        detail::CaseResult result;
        result.type = *gen::arbitrary<detail::CaseResult::Type>();
        result.description = *gen::arbitrary<std::string>();
        return result;
    }
};

template<>
class Arbitrary<detail::SuccessResult>
    : public gen::Generator<detail::SuccessResult>
{
public:
    detail::SuccessResult generate() const override
    {
        detail::SuccessResult result;
        result.numSuccess = *gen::positive<int>();
        return result;
    }
};

template<>
class Arbitrary<detail::FailureResult>
    : public gen::Generator<detail::FailureResult>
{
public:
    detail::FailureResult generate() const override
    {
        detail::FailureResult result;
        result.numSuccess = *gen::positive<int>();
        result.failingCase = *gen::arbitrary<detail::TestCase>();
        result.description = *gen::arbitrary<std::string>();
        result.numShrinks = *gen::positive<int>();
        result.counterExample = *gen::arbitrary<
            decltype(result.counterExample)>();
        return result;
    }
};

template<>
class Arbitrary<detail::GaveUpResult>
    : public gen::Generator<detail::GaveUpResult>
{
public:
    detail::GaveUpResult generate() const override
    {
        detail::GaveUpResult result;
        result.numSuccess = *gen::positive<int>();
        result.description = *gen::arbitrary<std::string>();
        return result;
    }
};

template<typename T>
class Arbitrary<Seq<T>> : public gen::Generator<Seq<T>>
{
public:
    Seq<T> generate() const override
    { return seq::fromContainer(*gen::arbitrary<std::vector<T>>()); }

    shrink::IteratorUP<Seq<T>> shrink(const Seq<T> &value) const
    {
        Seq<T> seq = value;
        std::vector<T> values;
        while (seq)
            values.push_back(seq.next());
        return shrink::map(
            gen::arbitrary<std::vector<T>>().shrink(std::move(values)),
            [](std::vector<T> &&x) { return seq::fromContainer(x); });
    }
};

template<typename T>
class Arbitrary<Maybe<T>> : public gen::Generator<Maybe<T>>
{
public:
    Maybe<T> generate() const override
    {
        if (*gen::noShrink(gen::ranged(0, 4)) == 0)
            return Maybe<T>();
        return Maybe<T>(*gen::arbitrary<T>());
    }

    shrink::IteratorUP<Maybe<T>> shrink(const Maybe<T> &value) const
    {
        if (!value)
            return shrink::nothing<T>();

        shrink::sequentially(
            shrink::constant({ Maybe<T>() }),
            shrink::map(
                gen::arbitrary<T>().shrink(value),
                [](T &&x) { return Maybe<T>(x); }));
    }
};

} // namespace rc
