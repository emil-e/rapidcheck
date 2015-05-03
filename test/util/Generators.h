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
    return gen::map(gen::positive<int>(),
                    [](int s) {
                      detail::SuccessResult result;
                      result.numSuccess = s;
                      return result;
                    });
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
inline Shrinkable<Maybe<T>> prependNothing(Shrinkable<Maybe<T>> &&s) {
  return shrinkable::mapShrinks(
      std::move(s),
      [](Seq<Shrinkable<Maybe<T>>> &&shrinks) {
        return seq::concat(seq::just(shrinkable::just(Maybe<T>())),
                           seq::map(std::move(shrinks), &prependNothing<T>));
      });
};

template <typename T>
struct Arbitrary<Maybe<T>> {
  static Gen<Maybe<T>> arbitrary() {
    return [](const Random &random, int size) {
      auto r = random;
      const auto x = r.split().next() % (size + 1);
      if (x == 0) {
        return shrinkable::just(Maybe<T>());
      }

      return prependNothing(
          shrinkable::map(gen::arbitrary<T>()(r, size),
                          [](T &&x) -> Maybe<T> { return std::move(x); }));
    };
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
