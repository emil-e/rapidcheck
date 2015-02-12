#pragma once

namespace rc {
namespace gen {

template<typename Generator, typename T>
SuchThat<Decay<Generator>, predicate::Not<predicate::Equals<Decay<T>>>>
distinctFrom(Generator &&generator, T &&value)
{
    return gen::suchThat(
        std::forward<Generator>(generator),
        predicate::Not<predicate::Equals<Decay<T>>>(std::forward<T>(value)));
}

template<typename T>
SuchThat<Arbitrary<Decay<T>>, predicate::Not<predicate::Equals<Decay<T>>>>
distinctFrom(T &&value)
{
    return gen::suchThat<Decay<T>>(
        predicate::Not<predicate::Equals<Decay<T>>>(std::forward<T>(value)));
}

} // namespace gen
} // namespace rc
