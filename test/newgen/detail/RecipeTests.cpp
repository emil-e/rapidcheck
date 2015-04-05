#include <catch.hpp>
#include <rapidcheck-catch.h>

#include "rapidcheck/newgen/detail/Recipe.h"
#include "rapidcheck/seq/Operations.h"
#include "rapidcheck/shrinkable/Transform.h"

#include "util/ArbitraryRandom.h"
#include "util/Generators.h"

using namespace rc;
using namespace rc::newgen::detail;

using Any = rc::detail::Any;

namespace rc {

template<>
struct NewArbitrary<Recipe>
{
    static Gen<Recipe> arbitrary()
    {
        return newgen::exec([]{
            Recipe recipe;
            recipe.random = *newgen::arbitrary<Random>();
            recipe.size = *newgen::inRange<int>(0, 200);
            std::size_t numIngredients = *newgen::inRange<std::size_t>(0, 5);
            recipe.ingredients = *newgen::container<Recipe::Ingredients>(
                numIngredients,
                newgen::map(
                    newgen::arbitrary<Shrinkable<int>>(),
                    [](Shrinkable<int> &&s) {
                        return shrinkable::map(std::move(s), &Any::of<int>);
                    }));
            recipe.numFixed = *newgen::inRange<std::size_t>(
                0, recipe.ingredients.size() + 1);
            return recipe;
        });
    }
};

} // namespace rc

namespace {

Shrinkable<int> mapToInt(Shrinkable<Any> shrinkable)
{
    return shrinkable::map(std::move(shrinkable), [](const Any &x) {
        return x.get<int>();
    });
}

bool equalsAsInt(Shrinkable<Any> lhs, Shrinkable<Any> rhs)
{
    return mapToInt(std::move(lhs)) == mapToInt(std::move(rhs));
}

bool equalIngredients(const Recipe &lhs, const Recipe &rhs)
{
    return std::equal(
        lhs.ingredients.begin(),
        lhs.ingredients.end(),
        rhs.ingredients.begin(),
        equalsAsInt);
}

} // namespace

TEST_CASE("shrinkRecipe") {
    newprop(
        "Random and size remain the same",
        [](const Recipe &recipe) {
            RC_ASSERT(
                seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                    return
                        (shrink.random == recipe.random) &&
                        (shrink.size == recipe.size);
                }));
        });

    newprop(
        "does not shrink fixed values",
        [](const Recipe &recipe) {
            RC_ASSERT(
                seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                    return std::equal(
                        recipe.ingredients.begin(),
                        recipe.ingredients.begin() + recipe.numFixed,
                        shrink.ingredients.begin(),
                        equalsAsInt);
                }));
        });

    newprop(
        "does not change any values except last one",
        [](const Recipe &recipe) {
            RC_ASSERT(
                seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                    auto size = shrink.ingredients.size() - 1;
                    return std::equal(
                        recipe.ingredients.begin(),
                        recipe.ingredients.begin() + size,
                        shrink.ingredients.begin(),
                        equalsAsInt);
                }));
        });

    newprop(
        "sets all values except last to fixed",
        [](const Recipe &recipe) {
            RC_ASSERT(
                seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                    return shrink.numFixed == (shrink.ingredients.size() - 1);
                }));
        });

    newprop(
        "last value is a shrink of the original value",
        [](const Recipe &recipe) {
            RC_ASSERT(
                seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                    std::size_t i = shrink.ingredients.size() - 1;
                    auto shrinks = recipe.ingredients[i].shrinks();
                    return seq::any(
                        std::move(shrinks),
                        [&](const Shrinkable<Any> &s) {
                            return equalsAsInt(s, shrink.ingredients[i]);
                        });
                }));
        });

    newprop(
        "yields empty sequence if all values are fixed",
        [](Recipe recipe) {
            recipe.numFixed = recipe.ingredients.size();
            RC_ASSERT(!shrinkRecipe(recipe).next());
        });

    newprop(
        "for any shrink of a non-fixed value, there is a shrink with that"
        " shrink",
        [] {
            // We want a recipe where there's actually something to shrink
            auto recipe = *newgen::suchThat<Recipe>([](const Recipe &r) {
                return !!shrinkRecipe(r).next();
            });

            std::size_t i = *newgen::suchThat(
                newgen::inRange<std::size_t>(
                    recipe.numFixed,
                    recipe.ingredients.size()),
                [&](std::size_t x) {
                    return seq::length(recipe.ingredients[x].shrinks()) > 0;
                });

            Seq<Shrinkable<Any>> shrinks = recipe.ingredients[i].shrinks();
            std::size_t numShrinks = seq::length(shrinks);
            std::size_t shrinkIndex = *newgen::inRange<std::size_t>(
                0, numShrinks);
            Shrinkable<Any> expectedShrink = *seq::at(shrinks, shrinkIndex);
            RC_ASSERT(seq::any(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                return
                    (shrink.ingredients.size() > i) &&
                    (equalsAsInt(shrink.ingredients[i], expectedShrink));
            }));
        });
}
