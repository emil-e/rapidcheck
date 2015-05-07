#pragma once

#include "rapidcheck/detail/FrequencyMap.h"

namespace rc {
namespace gen {
namespace detail {

template <typename Container>
class ElementOfGen {
public:
  using T = typename Container::value_type;

  template <typename Arg>
  ElementOfGen(Arg &&arg)
      : m_container(std::forward<Arg>(arg)) {}

  Shrinkable<T> operator()(const Random &random, int size) const {
    const auto start = begin(m_container);
    const auto containerSize = end(m_container) - start;
    if (containerSize == 0) {
      throw GenerationFailure("Cannot pick element from empty container.");
    }
    const auto i = Random(random).next() % containerSize;
    return shrinkable::just(*(start + i));
  }

private:
  Container m_container;
};

template <typename T>
class WeightedElementGen {
public:
  WeightedElementGen(std::vector<std::size_t> &&frequencies,
                     std::vector<T> &&elements)
      : m_map(std::move(frequencies))
      , m_elements(std::move(elements)) {}

  Shrinkable<T> operator()(const Random &random, int size) const {
    if (m_map.sum() == 0) {
      throw GenerationFailure("Sum of weights is 0");
    }
    return shrinkable::just(static_cast<T>(
        m_elements[m_map.lookup(Random(random).next() % m_map.sum())]));
  }

private:
  rc::detail::FrequencyMap m_map;
  std::vector<T> m_elements;
};

template <typename T>
class OneOfGen {
public:
  template <typename... Ts>
  OneOfGen(Gen<Ts>... gens)
      : m_gens{std::move(gens)...} {}

  Shrinkable<T> operator()(const Random &random, int size) const {
    Random r(random);
    const auto i = r.split().next() % m_gens.size();
    return m_gens[i](r, size);
  }

private:
  std::vector<Gen<T>> m_gens;
};

} // namespace detail

template <typename Container>
Gen<typename Decay<Container>::value_type> elementOf(Container &&container) {
  return detail::ElementOfGen<Decay<Container>>(
      std::forward<Container>(container));
}

template <typename T, typename... Ts>
Gen<Decay<T>> element(T &&element, Ts &&... elements) {
  using Vector = std::vector<Decay<T>>;
  return detail::ElementOfGen<Vector>(
      Vector{std::forward<T>(element), std::forward<Ts>(elements)...});
}

template <typename T>
Gen<T>
weightedElement(std::initializer_list<std::pair<std::size_t, T>> pairs) {
  std::vector<std::size_t> frequencies;
  frequencies.reserve(pairs.size());
  std::vector<T> elements;
  elements.reserve(pairs.size());

  for (auto &pair : pairs) {
    frequencies.push_back(pair.first);
    elements.push_back(std::move(pair.second));
  }

  return detail::WeightedElementGen<T>(std::move(frequencies),
                                       std::move(elements));
}

template <typename T, typename... Ts>
Gen<T> oneOf(Gen<T> gen, Gen<Ts>... gens) {
  return detail::OneOfGen<T>(std::move(gen), std::move(gens)...);
}

} // namespace gen
} // namespace rc
