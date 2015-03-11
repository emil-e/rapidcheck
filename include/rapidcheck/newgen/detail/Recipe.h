#pragma once

#include <vector>

#include "rapidcheck/Shrinkable.h"
#include "rapidcheck/detail/Any.h"
#include "rapidcheck/Random.h"

namespace rc {
namespace newgen {
namespace detail {

//! Describes the recipe for generating a certain value. This consists of vector
//! `Shrinkable<Any>` which are that sequence of value that were picked and a
//! number which describes the number of value that have already by exhaustively
//! shrunk. It also includes the `Random` generator to use and the size.
struct Recipe
{
    typedef std::vector<Shrinkable<rc::detail::Any>> Ingredients;

    Random random;
    int size = 0;
    Ingredients ingredients;
    std::size_t numFixed = 0;
};

//! Returns the non-recursive shrinks for the given recipe.
Seq<Recipe> shrinkRecipe(const Recipe &recipe);

} // namespace detail
} // namespace newgen
} // namespace rc
