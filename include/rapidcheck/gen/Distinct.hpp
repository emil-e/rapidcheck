#pragma once

namespace rc {
namespace gen {

template<typename Generator, typename T>
SuchThat<detail::DecayT<Generator>,
         predicate::Not<predicate::Equals<detail::DecayT<T>>>>
distinctFrom(Generator &&generator, T &&value)
{
    return gen::suchThat(
        std::forward<Generator>(generator),
        predicate::Not<predicate::Equals<detail::DecayT<T>>>(
            std::forward<T>(value)));
}

template<typename T>
SuchThat<Arbitrary<detail::DecayT<T>>,
         predicate::Not<predicate::Equals<detail::DecayT<T>>>>
distinctFrom(T &&value)
{
    return gen::suchThat<detail::DecayT<T>>(
        predicate::Not<predicate::Equals<detail::DecayT<T>>>(
            std::forward<T>(value)));
}

} // namespace gen
} // namespace rc
