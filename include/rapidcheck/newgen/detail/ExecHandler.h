#pragma once

#include "rapidcheck/newgen/detail/GenerationHandler.h"
#include "rapidcheck/newgen/detail/Recipe.h"

namespace rc {
namespace newgen {
namespace detail {

//! `GenerationHandler` used to implement `execRaw`.
class ExecHandler : public GenerationHandler
{
public:
    ExecHandler(Recipe &recipe);
    rc::detail::Any onGenerate(const Gen<rc::detail::Any> &gen);

private:
    Recipe &m_recipe;
    Random m_random;
    typedef Recipe::Ingredients::iterator Iterator;
    Iterator m_it;
};

} // namespace detail
} // namespace newgen
} // namespace rc
