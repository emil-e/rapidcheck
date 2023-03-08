#include "rapidcheck/gen/detail/Recipe.h"

#include "rapidcheck/seq/Transform.h"
#include "rapidcheck/detail/Any.h"

#include <stdexcept>

namespace rc {
namespace gen {
namespace detail {

Seq<Recipe> shrinkRecipe(const Recipe &recipe) {
  using Any = rc::detail::Any;


  return seq::mapcat(
      seq::range<std::size_t>(recipe.numFixed, recipe.ingredients.size()),
      [=](std::size_t i) {
        return seq::map(recipe.ingredients[i].shrinkable.shrinks(),
                        [=](Shrinkable<Any> &&shrink) {
                          Recipe shrunkRecipe(recipe);
  using difference_type = decltype(begin(shrunkRecipe.ingredients))::difference_type;
        if (i > std::numeric_limits<difference_type>::max()) {
            throw std::overflow_error("size() greater than the iterator difference type");
        }
                          const auto it = begin(shrunkRecipe.ingredients) + static_cast<difference_type>(i);
                          it->shrinkable = std::move(shrink);
                          shrunkRecipe.ingredients.erase(
                              it + 1, end(shrunkRecipe.ingredients));
                          shrunkRecipe.numFixed = i;
                          return shrunkRecipe;
                        });
      });
}

} // namespace detail
} // namespace gen
} // namespace rc
