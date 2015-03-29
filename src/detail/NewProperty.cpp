#include "rapidcheck/detail/NewProperty.h"

namespace rc {
namespace detail {

Gen<std::pair<CaseResult, Counterexample>> mapToResultPair(
    Gen<std::pair<CaseResult, newgen::detail::Recipe>> gen)
{
    return newgen::map(
        [](std::pair<CaseResult, newgen::detail::Recipe> &&p) {
            Counterexample example;
            const auto &ingr = p.second.ingredients;
            example.reserve(ingr.size());
            std::transform(
                begin(ingr), end(ingr), std::back_inserter(example),
                [](const Shrinkable<Any> &s) {
                    return s.value().describe();
                });

            return std::make_pair(std::move(p.first), std::move(example));
        }, std::move(gen));
}

} // namespace detail
} // namespace rc
