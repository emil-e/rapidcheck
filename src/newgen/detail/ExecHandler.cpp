#include "rapidcheck/newgen/detail/ExecHandler.h"

#include "rapidcheck/Gen.h"

namespace rc {
namespace newgen {
namespace detail {

ExecHandler::ExecHandler(Recipe &recipe)
    : m_recipe(recipe)
    , m_random(m_recipe.random)
    , m_it(begin(m_recipe.ingredients)) {}

rc::detail::Any ExecHandler::onGenerate(const Gen<rc::detail::Any> &gen)
{
    Random random = m_random.split();
    if (m_it == end(m_recipe.ingredients)) {
        m_it = m_recipe.ingredients.insert(
            m_it, gen(random, m_recipe.size));
    }
    auto current = m_it++;
    return current->value();
}

} // namespace detail
} // namespace newgen
} // namespace rc
