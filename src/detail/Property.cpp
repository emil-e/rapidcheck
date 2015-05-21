#include "rapidcheck/detail/Property.h"

#include <algorithm>

namespace rc {
namespace detail {

WrapperContext::WrapperContext(std::vector<std::string> &tags)
    : m_tags(tags) {}

void WrapperContext::addTag(std::string str) {
  m_tags.push_back(std::move(str));
}

namespace {

std::pair<std::string, std::string>
describeShrinkable(const Shrinkable<Any> &shrinkable) {
  // TODO I don't know if this is the right approach with counterexamples
  // even...
  try {
    return shrinkable.value().describe();
  } catch (const GenerationFailure &e) {
    return {"Generation failed", e.what()};
  } catch (const std::exception &e) {
    return {"Exception while generating", e.what()};
  } catch (...) {
    return {"Unknown exception", "<\?\?\?>"};
  }
}

} // namespace

Gen<CaseDescription>
mapToCaseDescription(Gen<std::pair<WrapperResult, gen::detail::Recipe>> gen) {
  return gen::map(std::move(gen),
                  [](std::pair<WrapperResult, gen::detail::Recipe> &&p) {
                    Example example;
                    const auto &ingr = p.second.ingredients;
                    example.reserve(ingr.size());
                    std::transform(begin(ingr),
                                   end(ingr),
                                   std::back_inserter(example),
                                   &describeShrinkable);

                    CaseDescription description;
                    description.result = std::move(p.first.result);
                    description.tags = std::move(p.first.tags);
                    description.example = std::move(example);
                    return description;
                  });
}

} // namespace detail
} // namespace rc
