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
class Arbitrary<Recipe> : public gen::Generator<Recipe>
{
public:
    Recipe generate() const override
    {
        Recipe recipe;
        recipe.random = *gen::arbitrary<Random>();
        recipe.size = *gen::ranged<int>(0, 200);
        std::size_t numIngredients = *gen::ranged<std::size_t>(0, 5);
        recipe.ingredients = *gen::vector<Recipe::Ingredients>(
            numIngredients,
            gen::map(gen::arbitrary<Shrinkable<int>>(),
                     [](Shrinkable<int> &&s) {
                         return shrinkable::map(&Any::of<int>, std::move(s));
                     }));
        recipe.numFixed = *gen::ranged<std::size_t>(
            0, recipe.ingredients.size());
        return recipe;
    }
};

} // namespace rc

namespace {

Shrinkable<int> mapToInt(Shrinkable<Any> shrinkable)
{
    return shrinkable::map([](const Any &x) { return x.get<int>(); },
                           std::move(shrinkable));
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
    prop("Random and size remain the same",
         [](const Recipe &recipe) {
             RC_ASSERT(
                 seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                     return
                         (shrink.random == recipe.random) &&
                         (shrink.size == recipe.size);
                 }));
         });

    prop("does not shrink fixed values",
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

    prop("does not change any values except last one",
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

    prop("sets all values except last to fixed",
         [](const Recipe &recipe) {
             RC_ASSERT(
                 seq::all(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                     return shrink.numFixed == (shrink.ingredients.size() - 1);
                 }));
         });

    prop("last value is a shrink of the original value",
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

    prop("yields empty sequence if all values are fixed",
         [](Recipe recipe) {
             recipe.numFixed = recipe.ingredients.size();
             RC_ASSERT(!shrinkRecipe(recipe).next());
         });

    prop("for any shrink of a non-fixed value, there is a shrink with that"
         " shrink",
         [] {
             // We want a recipe where there's actually something to shrink
             auto recipe = *gen::suchThat<Recipe>([](const Recipe &r) {
                 return !!shrinkRecipe(r).next();
             });

             std::size_t i = *gen::suchThat(
                 gen::ranged<std::size_t>(
                     recipe.numFixed,
                     recipe.ingredients.size()),
                 [&](std::size_t x) {
                     return seq::length(recipe.ingredients[x].shrinks()) > 0;
                 });

             Seq<Shrinkable<Any>> shrinks = recipe.ingredients[i].shrinks();
             std::size_t numShrinks = seq::length(shrinks);
             std::size_t shrinkIndex = *gen::ranged<std::size_t>(
                 0, numShrinks);
             Shrinkable<Any> expectedShrink = *seq::at(shrinks, shrinkIndex);
             RC_ASSERT(seq::any(shrinkRecipe(recipe), [&](const Recipe &shrink) {
                 return
                     (shrink.ingredients.size() > i) &&
                     (equalsAsInt(shrink.ingredients[i], expectedShrink));
             }));
         });
}
