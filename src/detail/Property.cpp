#include "rapidcheck/detail/Property.h"

#include <algorithm>

namespace rc {
namespace detail {
namespace {

std::pair<std::string, std::string> describeShrinkable(
    const Shrinkable<Any> &shrinkable)
{
    // TODO I don't know if this is the right approach with counterexamples
    // even...
    try {
        return shrinkable.value().describe();
    } catch (const GenerationFailure &e) {
        return {"Generation failed", e.what()};
    } catch (const std::exception &e) {
        return {"Exception wil generating", e.what()};
    } catch (...) {
        return {"Unknown exception", "<\?\?\?>"};
    }
}

} // namespace

Gen<CaseDescription> mapToCaseDescription(
    Gen<std::pair<CaseResult, gen::detail::Recipe>> gen)
{
    return gen::map(
        std::move(gen),
        [](std::pair<CaseResult, gen::detail::Recipe> &&p) {
            Example example;
            const auto &ingr = p.second.ingredients;
            example.reserve(ingr.size());
            std::transform(
                begin(ingr), end(ingr),
                std::back_inserter(example),
                &describeShrinkable);


            CaseDescription description;
            description.result = std::move(p.first);
            description.example = std::move(example);
            return description;
        });
}

} // namespace detail
} // namespace rc
