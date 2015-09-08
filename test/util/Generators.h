#pragma once

#include "rapidcheck/detail/Configuration.h"
#include "rapidcheck/Seq.h"
#include "rapidcheck/Shrinkable.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/Maybe.h"
#include "rapidcheck/seq/Create.h"

namespace rc {

template <>
struct Arbitrary<detail::TestParams> {
  static Gen<detail::TestParams> arbitrary() {
    return gen::build<detail::TestParams>(
        gen::set(&detail::TestParams::seed),
        gen::set(&detail::TestParams::maxSuccess, gen::inRange(0, 100)),
        gen::set(&detail::TestParams::maxSize, gen::inRange(0, 101)),
        gen::set(&detail::TestParams::maxDiscardRatio, gen::inRange(0, 100)),
        gen::set(&detail::TestParams::disableShrinking),
        gen::set(&detail::TestParams::shrinkTries, gen::inRange(1,100)));
  }
};

template <>
struct Arbitrary<detail::Configuration> {
  static Gen<detail::Configuration> arbitrary() {
    return gen::build<detail::Configuration>(
      gen::set(&detail::Configuration::testParams),
      gen::set(&detail::Configuration::verboseProgress),
      gen::set(&detail::Configuration::verboseShrinking));
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
        gen::set(&detail::FailureResult::description),
        gen::set(&detail::FailureResult::numShrinks, gen::positive<int>()),
        gen::set(&detail::FailureResult::counterExample));
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
struct Arbitrary<detail::CaseDescription> {
  static Gen<detail::CaseDescription> arbitrary() {
    return gen::build<detail::CaseDescription>(
        gen::set(&detail::CaseDescription::result),
        gen::set(&detail::CaseDescription::tags),
        gen::set(&detail::CaseDescription::example,
                 gen::map<detail::Example>(
                     [](detail::Example &&example)
                         -> std::function<detail::Example()> {
                           return fn::constant(std::move(example));
                         })));
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
