#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>

#include "rapidcheck/gen/detail/Recipe.h"
#include "rapidcheck/seq/Operations.h"
#include "rapidcheck/shrinkable/Transform.h"

#include "util/ArbitraryRandom.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::gen::detail;

using Any = rc::detail::Any;

namespace rc {

template <>
struct Arbitrary<Recipe::Ingredient> {
  static Gen<Recipe::Ingredient> arbitrary() {
    return gen::construct<Recipe::Ingredient>(
        gen::arbitrary<std::string>(),
        gen::map<Shrinkable<int>>([](Shrinkable<int> &&s) {
          return shrinkable::map(std::move(s), &Any::of<int>);
        }));
  }
};

template <>
struct Arbitrary<Recipe> {
  static Gen<Recipe> arbitrary() {
    return gen::exec([] {
      Recipe recipe;
      recipe.random = *gen::arbitrary<Random>();
      recipe.size = *gen::inRange<int>(0, 200);
      const auto numIngredients = *gen::inRange<std::size_t>(0, 5);
      recipe.ingredients = *gen::container<Recipe::Ingredients>(
          numIngredients, gen::arbitrary<Recipe::Ingredient>());
      recipe.numFixed =
          *gen::inRange<std::size_t>(0, recipe.ingredients.size() + 1);
      return recipe;
    });
  }
};

} // namespace rc

namespace {

Shrinkable<int> mapToInt(Shrinkable<Any> shrinkable) {
  return shrinkable::map(std::move(shrinkable),
                         [](const Any &x) { return x.get<int>(); });
}

bool equalAsInt(Shrinkable<Any> lhs, Shrinkable<Any> rhs) {
  return mapToInt(std::move(lhs)) == mapToInt(std::move(rhs));
}

bool equalIntIngredients(const Recipe::Ingredient &lhs,
                         const Recipe::Ingredient &rhs) {
  return (lhs.description == rhs.description) &&
      equalAsInt(lhs.shrinkable, rhs.shrinkable);
}

} // namespace

TEST_CASE("shrinkRecipe") {
  prop("Random and size remain the same",
       [](const Recipe &recipe) {
         RC_ASSERT(seq::all(shrinkRecipe(recipe),
                            [&](const Recipe &shrink) {
                              return (shrink.random == recipe.random) &&
                                  (shrink.size == recipe.size);
                            }));
       });

  prop("does not shrink fixed values",
       [](const Recipe &recipe) {
         RC_ASSERT(seq::all(shrinkRecipe(recipe),
                            [&](const Recipe &shrink) {
                              return std::equal(recipe.ingredients.begin(),
                                                recipe.ingredients.begin() +
                                                    recipe.numFixed,
                                                shrink.ingredients.begin(),
                                                equalIntIngredients);
                            }));
       });

  prop("does not change any values except last one",
       [](const Recipe &recipe) {
         RC_ASSERT(seq::all(shrinkRecipe(recipe),
                            [&](const Recipe &shrink) {
                              const auto size = shrink.ingredients.size() - 1;
                              return std::equal(recipe.ingredients.begin(),
                                                recipe.ingredients.begin() +
                                                    size,
                                                shrink.ingredients.begin(),
                                                equalIntIngredients);
                            }));
       });

  prop("sets all values except last to fixed",
       [](const Recipe &recipe) {
         RC_ASSERT(seq::all(shrinkRecipe(recipe),
                            [&](const Recipe &shrink) {
                              return shrink.numFixed ==
                                  (shrink.ingredients.size() - 1);
                            }));
       });

  prop("last value is a shrink of the original value",
       [](const Recipe &recipe) {
         RC_ASSERT(seq::all(shrinkRecipe(recipe),
                            [&](const Recipe &shrink) {
                              const auto i = shrink.ingredients.size() - 1;
                              auto shrinks =
                                  recipe.ingredients[i].shrinks();
                              return seq::any(
                                  std::move(shrinks),
                                  [&](const Shrinkable<Any> &s) {
                                    return equalAsInt(
                                        s, shrink.ingredients[i].shrinkable);
                                  });
                            }));
       });

  prop("maintains description of shrunk ingredient",
       [](const Recipe &recipe) {
         RC_ASSERT(seq::all(shrinkRecipe(recipe),
                            [&](const Recipe &shrink) {
                              const auto i = shrink.ingredients.size() - 1;
                              return (shrink.ingredients[i].description ==
                                      recipe.ingredients[i].description);
                            }));
       });

  prop("yields empty sequence if all values are fixed",
       [](Recipe recipe) {
         recipe.numFixed = recipe.ingredients.size();
         RC_ASSERT(!shrinkRecipe(recipe).next());
       });

  prop(
      "for any shrink of a non-fixed value, there is a shrink where that value "
      "has been shrunk",
      [] {
        // We want a recipe where there's actually something to shrink
        const auto recipe = *gen::suchThat<Recipe>([](const Recipe &r) {
          return !!shrinkRecipe(r).next();
        });

        const auto i = *gen::suchThat(
            gen::inRange<std::size_t>(recipe.numFixed,
                                      recipe.ingredients.size()),
            [&](std::size_t x) {
              return bool(recipe.ingredients[x].shrinks().next());
            });

        const auto shrinks = recipe.ingredients[i].shrinks();
        const auto numShrinks = seq::length(shrinks);
        const auto shrinkIndex = *gen::inRange<std::size_t>(0, numShrinks);
        const auto expectedShrink = *seq::at(shrinks, shrinkIndex);
        RC_ASSERT(
            seq::any(shrinkRecipe(recipe),
                     [&](const Recipe &shrink) {
                       return (shrink.ingredients.size() > i) &&
                           (equalAsInt(shrink.ingredients[i].shrinkable, expectedShrink));
                     }));
      });
}
