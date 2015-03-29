#include "rapidcheck/detail/NewProperty.h"

namespace rc {
namespace detail {

Gen<CaseDescription> mapToCaseDescription(
    Gen<std::pair<CaseResult, newgen::detail::Recipe>> gen)
{
    return newgen::map(
        [](std::pair<CaseResult, newgen::detail::Recipe> &&p) {
            Example example;
            const auto &ingr = p.second.ingredients;
            example.reserve(ingr.size());
            std::transform(
                begin(ingr), end(ingr), std::back_inserter(example),
                [](const Shrinkable<Any> &s) {
                    return s.value().describe();
                });

            CaseDescription description;
            description.result = std::move(p.first);
            description.example = std::move(example);
            return description;
        }, std::move(gen));
}

} // namespace detail
} // namespace rc
