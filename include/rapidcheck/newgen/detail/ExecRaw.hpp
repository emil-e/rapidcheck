#pragma once

#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/newgen/Tuple.h"
#include "rapidcheck/newgen/detail/ExecHandler.h"

namespace rc {
namespace newgen {
namespace detail {

template<typename Callable>
std::pair<Decay<rc::detail::ReturnType<Callable>>, Recipe>
execWithRecipe(Callable callable, Recipe recipe)
{
    using namespace rc::detail;
    Recipe resultRecipe(recipe);
    ExecHandler handler(resultRecipe);
    ImplicitParam<param::CurrentHandler> letHandler(&handler);

    using ArgsTuple = tl::ToTuple<tl::DecayAll<ArgTypes<Callable>>>;
    return std::make_pair(
        applyTuple(*newgen::arbitrary<ArgsTuple>(), callable),
        std::move(resultRecipe));
}

template<typename Callable>
Seq<Shrinkable<std::pair<Decay<rc::detail::ReturnType<Callable>>, Recipe>>>
shrinksOfRecipe(Callable callable, Recipe recipe)
{
    return seq::map(
        [=](Recipe &&shrunkRecipe) {
            return shrinkableWithRecipe(callable, shrunkRecipe);
        }, shrinkRecipe(std::move(recipe)));
}

template<typename Callable>
Shrinkable<std::pair<Decay<rc::detail::ReturnType<Callable>>, Recipe>>
shrinkableWithRecipe(Callable callable, Recipe recipe)
{
    typedef rc::detail::ReturnType<Callable> T;
    return shrinkable::shrink(
        [=] { return execWithRecipe(callable, recipe); },
        [=](std::pair<T, Recipe> &&p) {
            return shrinksOfRecipe(callable, std::move(p.second));
        });
}

template<typename Callable>
Gen<std::pair<Decay<rc::detail::ReturnType<Callable>>, Recipe>>
execRaw(Callable callable)
{
    return [=](const Random &random, int size) {
        Recipe recipe;
        recipe.random = random;
        recipe.size = size;
        return shrinkableWithRecipe(callable, recipe);
    };
}

} // namespace detail
} // namespace newgen
} // namespace rc
