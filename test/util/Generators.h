#pragma once

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Seq.h"
#include "rapidcheck/Shrinkable.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/Maybe.h"
#include "rapidcheck/seq/Create.h"

namespace rc {

template <>
struct Arbitrary<detail::Configuration> {
  static Gen<detail::Configuration> arbitrary() {
    return gen::build<detail::Configuration>(
        gen::set(&detail::Configuration::seed),
        gen::set(&detail::Configuration::maxSuccess,
                 gen::inRange<int>(0, 1000)),
        gen::set(&detail::Configuration::maxSize, gen::inRange<int>(0, 1000)),
        gen::set(&detail::Configuration::maxDiscardRatio,
                 gen::inRange<int>(0, 100)));
  }
};

template <>
struct Arbitrary<detail::TestCase> {
  static Gen<detail::TestCase> arbitrary() {
    return gen::build<detail::TestCase>(gen::set(&detail::TestCase::size,
                                                 gen::withSize([](int size) {
                                                   // TODO this should be
                                                   // replaced by a sized ranged
                                                   // generator
                                                   // instead
                                                   return gen::inRange<int>(
                                                       0, size + 1);
                                                 })),
                                        gen::set(&detail::TestCase::seed));
  }
};

template <>
struct Arbitrary<detail::CaseResult::Type> {
  static Gen<detail::CaseResult::Type> arbitrary() {
    return gen::element(detail::CaseResult::Type::Success,
                        detail::CaseResult::Type::Failure,
                        detail::CaseResult::Type::Discard);
  }
};

template <>
struct Arbitrary<detail::CaseResult> {
  static Gen<detail::CaseResult> arbitrary() {
    return gen::build<detail::CaseResult>(
        gen::set(&detail::CaseResult::type),
        gen::set(&detail::CaseResult::description));
  }
};

template <>
struct Arbitrary<detail::SuccessResult> {
  static Gen<detail::SuccessResult> arbitrary() {
    return gen::build<detail::SuccessResult>(
        gen::set(&detail::SuccessResult::numSuccess, gen::positive<int>()),
        gen::set(&detail::SuccessResult::distribution,
                 gen::container<detail::Distribution>(
                     gen::scale(0.1, gen::arbitrary<detail::Tags>()),
                     gen::arbitrary<int>())));
  }
};

template <>
struct Arbitrary<detail::FailureResult> {
  static Gen<detail::FailureResult> arbitrary() {
    return gen::build<detail::FailureResult>(
        gen::set(&detail::FailureResult::numSuccess, gen::positive<int>()),
        gen::set(&detail::FailureResult::failingCase),
        gen::set(&detail::FailureResult::description),
        gen::set(&detail::FailureResult::numShrinks, gen::positive<int>()));
  }
};

template <>
struct Arbitrary<detail::GaveUpResult> {
  static Gen<detail::GaveUpResult> arbitrary() {
    return gen::build<detail::GaveUpResult>(
        gen::set(&detail::GaveUpResult::numSuccess, gen::positive<int>()),
        gen::set(&detail::GaveUpResult::description));
  }
};

template <>
struct Arbitrary<detail::TestParams> {
  static Gen<detail::TestParams> arbitrary() {
    return gen::build<detail::TestParams>(
        gen::set(&detail::TestParams::maxSuccess, gen::inRange(0, 100)),
        gen::set(&detail::TestParams::maxSize, gen::inRange(0, 101)),
        gen::set(&detail::TestParams::maxDiscardRatio, gen::inRange(0, 100)));
  }
};

template <typename T>
struct Arbitrary<Seq<T>> {
  static Gen<Seq<T>> arbitrary() {
    return gen::map<std::vector<T>>(&seq::fromContainer<std::vector<T>>);
  }
};

template <typename T>
struct Arbitrary<Shrinkable<T>> {
  static Gen<Shrinkable<T>> arbitrary() {
    // TODO fapply
    return gen::map(
        gen::pair(
            gen::arbitrary<T>(),
            gen::scale(0.25, gen::lazy(&gen::arbitrary<Seq<Shrinkable<T>>>))),
        [](std::pair<T, Seq<Shrinkable<T>>> &&p) {
          return shrinkable::just(std::move(p.first), std::move(p.second));
        });
  }
};

} // namespace rc
