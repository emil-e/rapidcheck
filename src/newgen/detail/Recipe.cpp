#include "rapidcheck/newgen/detail/Recipe.h"

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/detail/Any.h"

namespace rc {
namespace newgen {
namespace detail {

Seq<Recipe> shrinkRecipe(const Recipe &recipe)
{
    using Any = rc::detail::Any;

    return seq::mapcat(
        seq::range<size_t>(recipe.numFixed, recipe.ingredients.size()),
        [=](std::size_t i) {
            return seq::map(
                recipe.ingredients[i].shrinks(),
                [=](Shrinkable<Any> &&shrink) {
                    Recipe shrunkRecipe(recipe);
                    const auto it = begin(shrunkRecipe.ingredients) + i;
                    *it = std::move(shrink);
                    shrunkRecipe.ingredients.erase(
                        it + 1, end(shrunkRecipe.ingredients));
                    shrunkRecipe.numFixed = i;
                    return shrunkRecipe;
                });
        });
}

} // namespace detail
} // namespace newgen
} // namespace rc
